// TurnBasedGameMode.cpp
#include "TurnBasedGameMode.h"
#include "ChessBoard.h"
#include "ChessPieceBase.h"
#include "PlayerChessPiece.h"
#include "ChessTile.h"
#include "PowerUp.h"
#include "ChessPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ATurnBasedGameMode::ATurnBasedGameMode()
{
    PlayerControllerClass = AChessPlayerController::StaticClass();
    DefaultPawnClass = APlayerChessPiece::StaticClass();
}

void ATurnBasedGameMode::BeginPlay()
{
    Super::BeginPlay();

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

    // Spawn player piece
    FVector StartLocation = GameBoard->GetWorldLocationForTile(GameBoard->BoardWidth / 2, GameBoard->BoardHeight / 2);
    FRotator StartRotation = FRotator::ZeroRotator;
    FActorSpawnParameters SpawnParams;

    PlayerPiece = GetWorld()->SpawnActor<APlayerChessPiece>(APlayerChessPiece::StaticClass(), StartLocation, StartRotation, SpawnParams);

    if (PlayerPiece)
    {
        PlayerPiece->GridX = GameBoard->BoardWidth / 2;
        PlayerPiece->GridY = GameBoard->BoardHeight / 2;

        AChessTile* StartTile = GameBoard->GetTileAt(PlayerPiece->GridX, PlayerPiece->GridY);
        if (StartTile)
        {
            StartTile->OccupyingPiece = PlayerPiece;
        }

        AllPieces.Add(PlayerPiece);

        // Possess the player piece
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC)
        {
            PC->Possess(PlayerPiece);
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
            FString::Printf(TEXT("========== Turn %d - Player's Turn =========="), CurrentTurn));
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

void ATurnBasedGameMode::EndPlayerTurn()
{
    if (!bPlayerTurn)
    {
        return;
    }

    bPlayerTurn = false;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Orange,
            TEXT("Player ended turn. Enemy turn starting..."));
    }

    // Execute enemy turns
    ExecuteEnemyTurns();

    // Start next turn
    StartNextTurn();
}

void ATurnBasedGameMode::ExecuteEnemyTurns()
{
    if (!GameBoard || !PlayerPiece)
    {
        return;
    }

    for (AChessPieceBase* Enemy : AllPieces)
    {
        if (!Enemy || Enemy == PlayerPiece || Enemy->bHasActedThisTurn)
        {
            continue;
        }

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
                    TEXT("Enemy attacked player!"));
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
    }
}

void ATurnBasedGameMode::SpawnRandomEnemies(int32 Count)
{
    if (!GameBoard)
    {
        return;
    }

    for (int32 i = 0; i < Count; i++)
    {
        // Find random empty tile
        int32 RandomX = FMath::RandRange(0, GameBoard->BoardWidth - 1);
        int32 RandomY = FMath::RandRange(0, GameBoard->BoardHeight - 1);

        AChessTile* Tile = GameBoard->GetTileAt(RandomX, RandomY);

        if (Tile && !Tile->OccupyingPiece)
        {
            FVector SpawnLocation = GameBoard->GetWorldLocationForTile(RandomX, RandomY);
            FRotator SpawnRotation = FRotator::ZeroRotator;
            FActorSpawnParameters SpawnParams;

            AChessPieceBase* Enemy = GetWorld()->SpawnActor<AChessPieceBase>(AChessPieceBase::StaticClass(), SpawnLocation, SpawnRotation, SpawnParams);

            if (Enemy)
            {
                Enemy->GridX = RandomX;
                Enemy->GridY = RandomY;
                Enemy->PieceType = static_cast<EPieceType>(FMath::RandRange(1, 3)); // Random enemy type

                Tile->OccupyingPiece = Enemy;
                AllPieces.Add(Enemy);
            }
        }
        else
        {
            // Try again if tile was occupied
            i--;
        }
    }
}

void ATurnBasedGameMode::SpawnRandomPowerUps(int32 Count)
{
    if (!GameBoard)
    {
        return;
    }

    for (int32 i = 0; i < Count; i++)
    {
        int32 RandomX = FMath::RandRange(0, GameBoard->BoardWidth - 1);
        int32 RandomY = FMath::RandRange(0, GameBoard->BoardHeight - 1);

        AChessTile* Tile = GameBoard->GetTileAt(RandomX, RandomY);

        if (Tile && !Tile->OccupyingPiece)
        {
            FVector SpawnLocation = GameBoard->GetWorldLocationForTile(RandomX, RandomY);
            SpawnLocation.Z += 50.0f; // Slightly above the board
            FRotator SpawnRotation = FRotator::ZeroRotator;
            FActorSpawnParameters SpawnParams;

            APowerUp* PowerUp = GetWorld()->SpawnActor<APowerUp>(APowerUp::StaticClass(), SpawnLocation, SpawnRotation, SpawnParams);

            // Random power-up type
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