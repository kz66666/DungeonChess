// TurnBasedGameMode.cpp
#include "TurnBasedGameMode.h"
#include "ChessBoard.h"
#include "ChessPieceBase.h"
#include "PlayerChessPiece.h"
#include "ChessTile.h"
#include "PowerUp.h"
#include "RookChessPiece.h"
#include "KnightChessPiece.h"
#include "BishopChessPiece.h"
#include "ChessPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

ATurnBasedGameMode::ATurnBasedGameMode()
{
    PlayerControllerClass = AChessPlayerController::StaticClass();
    DefaultPawnClass = APlayerChessPiece::StaticClass();
}

void ATurnBasedGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Wait a frame for everything to initialize
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            InitializeGame();
        }, 0.1f, false);
}

void ATurnBasedGameMode::InitializeGame()
{
    // Find the game board in the level
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AChessBoard::StaticClass(), FoundActors);

    if (FoundActors.Num() > 0)
    {
        GameBoard = Cast<AChessBoard>(FoundActors[0]);
    }

    if (!GameBoard)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("ERROR: No ChessBoard found in level!"));
        }
        return;
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Chess Board Found!"));
    }

    // Find the player pawn
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC)
    {
        PlayerPiece = Cast<APlayerChessPiece>(PC->GetPawn());

        if (PlayerPiece)
        {
            // Position player at center of board
            int32 StartX = GameBoard->BoardWidth / 2;
            int32 StartY = GameBoard->BoardHeight / 2;

            FVector StartLocation = GameBoard->GetWorldLocationForTile(StartX, StartY);
            StartLocation.Z = 100.0f;

            PlayerPiece->SetActorLocation(StartLocation);
            PlayerPiece->GridX = StartX;
            PlayerPiece->GridY = StartY;

            AChessTile* StartTile = GameBoard->GetTileAt(StartX, StartY);
            if (StartTile)
            {
                StartTile->OccupyingPiece = PlayerPiece;
            }

            AllPieces.Add(PlayerPiece);

            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
                    FString::Printf(TEXT("Player spawned at (%d, %d)"), StartX, StartY));
            }
        }
    }

    // Spawn enemies and power-ups
    SpawnRandomEnemies(5);
    SpawnRandomPowerUps(3);

    StartNextTurn();
}

void ATurnBasedGameMode::StartNextTurn()
{
    CurrentTurn++;
    bPlayerTurn = true;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
            FString::Printf(TEXT("===== TURN %d - Your Turn ====="), CurrentTurn));
    }

    // Reset all pieces
    for (AChessPieceBase* Piece : AllPieces)
    {
        if (Piece)
        {
            Piece->OnTurnStart();
        }
    }
}

void ATurnBasedGameMode::OnPlayerAction()
{
    if (!bPlayerTurn)
    {
        return;
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
            TEXT("Player action complete! Enemy turn starting..."));
    }

    bPlayerTurn = false;

    // Small delay before enemy turns start
    FTimerHandle DelayTimer;
    GetWorld()->GetTimerManager().SetTimer(DelayTimer, [this]()
        {
            ExecuteEnemyTurns();
        }, 0.5f, false);
}

void ATurnBasedGameMode::ExecuteEnemyTurns()
{
    if (!GameBoard || !PlayerPiece)
    {
        StartNextTurn();
        return;
    }

    // Reset enemy index
    CurrentEnemyIndex = 0;

    // Start processing enemy turns one by one
    ProcessEnemyTurn();
}

void ATurnBasedGameMode::ProcessEnemyTurn()
{
    // Find next valid enemy
    while (CurrentEnemyIndex < AllPieces.Num())
    {
        AChessPieceBase* Enemy = AllPieces[CurrentEnemyIndex];
        CurrentEnemyIndex++;

        if (!Enemy || Enemy == PlayerPiece || Enemy->bHasActedThisTurn)
        {
            continue;
        }

        // This is a valid enemy that hasn't acted
        // Simple AI: Move towards player or attack if adjacent
        TArray<FIntPoint> AttackTiles = Enemy->GetAttackTiles(GameBoard);

        // Check if player is in attack range
        bool bCanAttackPlayer = false;
        for (const FIntPoint& Tile : AttackTiles)
        {
            if (Tile.X == PlayerPiece->GridX && Tile.Y == PlayerPiece->GridY)
            {
                bCanAttackPlayer = true;
                break;
            }
        }

        if (bCanAttackPlayer)
        {
            // Attack player
            Enemy->AttackPiece(PlayerPiece);
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red,
                    TEXT("Enemy attacked you!"));
            }
        }
        else
        {
            // Move towards player
            TArray<FIntPoint> ValidMoves = Enemy->GetValidMoves(GameBoard);

            if (ValidMoves.Num() > 0)
            {
                // Find the move that gets closest to player
                FIntPoint BestMove = ValidMoves[0];
                float BestDistance = FLT_MAX;

                for (const FIntPoint& Move : ValidMoves)
                {
                    float Distance = FVector2D::Distance(
                        FVector2D(Move.X, Move.Y),
                        FVector2D(PlayerPiece->GridX, PlayerPiece->GridY)
                    );

                    if (Distance < BestDistance)
                    {
                        BestDistance = Distance;
                        BestMove = Move;
                    }
                }

                Enemy->MoveToPiece(BestMove.X, BestMove.Y, GameBoard);
            }
        }

        // Schedule next enemy turn after a delay
        GetWorld()->GetTimerManager().SetTimer(
            EnemyTurnTimerHandle,
            this,
            &ATurnBasedGameMode::ProcessEnemyTurn,
            0.5f,  // 0.5 second delay between enemy actions
            false
        );

        return; // Exit and wait for timer
    }

    // All enemies have acted, start next player turn
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green,
            TEXT("Enemy turn complete!"));
    }

    FTimerHandle DelayTimer;
    GetWorld()->GetTimerManager().SetTimer(DelayTimer, [this]()
        {
            StartNextTurn();
        }, 1.0f, false);
}

void ATurnBasedGameMode::SpawnRandomEnemies(int32 Count)
{
    if (!GameBoard)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("No GameBoard for enemy spawn!"));
        }
        return;
    }

    /*if (!EnemyPieceClass)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("No EnemyPieceClass set in GameMode!"));
        }
        return;
    }*/

    /*TArray<TSubclassOf<AChessPieceBase>> EnemyClasses = { ARookChessPiece::StaticClass(),
                                                     AKnightChessPiece::StaticClass(),
                                                     ABishopChessPiece::StaticClass() };*/

    TArray<EPieceType> EnemyTypes = {
        EPieceType::EnemyRook,
        EPieceType::EnemyKnight,
        EPieceType::EnemyBishop
    };



    int32 SpawnedCount = 0;
    int32 Attempts = 0;
    int32 MaxAttempts = Count * 10;

    while (SpawnedCount < Count && Attempts < MaxAttempts)
    {
        Attempts++;

        int32 RandomX = FMath::RandRange(0, GameBoard->BoardWidth - 1);
        int32 RandomY = FMath::RandRange(0, GameBoard->BoardHeight - 1);

		/*GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue,
			FString::Printf(TEXT("Trying to spawn enemy at (%d, %d)"), RandomX, RandomY));*/

        // Skip if too close to player spawn
        int32 CenterX = GameBoard->BoardWidth / 2;
        int32 CenterY = GameBoard->BoardHeight / 2;
        if (FMath::Abs(RandomX - CenterX) <= 1 && FMath::Abs(RandomY - CenterY) <= 1)
        {
            continue;
        }

        AChessTile* Tile = GameBoard->GetTileAt(RandomX, RandomY);

        if (Tile && !Tile->OccupyingPiece)
        {
            // Use the tile's actual location instead of recalculating
            FVector SpawnLocation = Tile->GetActorLocation();
            SpawnLocation.Z = 100.0f; // Same height as player
            FRotator SpawnRotation = FRotator::ZeroRotator;
            FActorSpawnParameters SpawnParams;

            int32 Index = FMath::RandRange(0, EnemyPieceClasses.Num() - 1);

            /*GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange,
                FString::Printf(TEXT("Trying to spawn class: %s"), *EnemyClasses[Index]->GetName()));

            GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan,
                FString::Printf(TEXT("Class %s is abstract? %s"),
                    *EnemyClasses[Index]->GetName(),
                    EnemyClasses[Index]->HasAnyClassFlags(CLASS_Abstract) ? TEXT("Yes") : TEXT("No")));*/



            AChessPieceBase* Enemy = GetWorld()->SpawnActor<AChessPieceBase>(
                EnemyPieceClasses[Index],
                SpawnLocation,
                SpawnRotation,
                SpawnParams
            );

            if (Enemy)
            {
                Enemy->GridX = RandomX;
                Enemy->GridY = RandomY;
                /*Enemy->PieceType = static_cast<EPieceType>(FMath::RandRange(1, 3));*/
                Enemy->PieceType = EnemyTypes[Index];

                Tile->OccupyingPiece = Enemy;
                AllPieces.Add(Enemy);
                SpawnedCount++;

                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange,
                        FString::Printf(TEXT("Enemy %d spawned at grid (%d, %d), world (%.1f, %.1f)"),
                            SpawnedCount, RandomX, RandomY, SpawnLocation.X, SpawnLocation.Y));
                }
            }
            else {
                GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red,
                    FString::Printf(TEXT("IT'S NOT AN ENEMY")));
            }
        }
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
            FString::Printf(TEXT("Spawned %d enemies"), SpawnedCount));
    }
}

void ATurnBasedGameMode::SpawnRandomPowerUps(int32 Count)
{
    if (!GameBoard)
    {
        return;
    }

    // Determine which class to spawn
    TSubclassOf<APowerUp> ClassToSpawn;
    if (PowerUpClass)
    {
        ClassToSpawn = PowerUpClass;
    }
    else
    {
        ClassToSpawn = APowerUp::StaticClass();
    }

    for (int32 i = 0; i < Count; i++)
    {
        int32 RandomX = FMath::RandRange(0, GameBoard->BoardWidth - 1);
        int32 RandomY = FMath::RandRange(0, GameBoard->BoardHeight - 1);

        AChessTile* Tile = GameBoard->GetTileAt(RandomX, RandomY);

        if (Tile && !Tile->OccupyingPiece)
        {
            FVector SpawnLocation = GameBoard->GetWorldLocationForTile(RandomX, RandomY);
            SpawnLocation.Z = 50.0f;
            FRotator SpawnRotation = FRotator::ZeroRotator;
            FActorSpawnParameters SpawnParams;

            APowerUp* PowerUp = GetWorld()->SpawnActor<APowerUp>(ClassToSpawn, SpawnLocation, SpawnRotation, SpawnParams);

            if (PowerUp)
            {
                PowerUp->PowerUpType = static_cast<EPowerUpType>(FMath::RandRange(0, 3));
            }
        }
        else
        {
            i--;
        }
    }
}