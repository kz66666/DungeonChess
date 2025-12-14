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
            // Determine spawn position
            float StartX, StartY;

            if (bRandomPlayerSpawn)
            {
                // Random spawn anywhere on the board (center of random tile)
                StartX = FMath::RandRange(0, GameBoard->BoardWidth - 1) + 0.5f;
                StartY = FMath::RandRange(0, GameBoard->BoardHeight - 1) + 0.5f;

                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
                        FString::Printf(TEXT("Random spawn at (%.1f, %.1f)"), StartX, StartY));
                }
            }
            else
            {
                // Use configured spawn position
                StartX = PlayerStartX;
                StartY = PlayerStartY;

                // Clamp to valid board range
                StartX = FMath::Clamp(StartX, 0.0f, static_cast<float>(GameBoard->BoardWidth) - 0.01f);
                StartY = FMath::Clamp(StartY, 0.0f, static_cast<float>(GameBoard->BoardHeight) - 0.01f);

                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
                        FString::Printf(TEXT("Fixed spawn at (%.1f, %.1f)"), StartX, StartY));
                }
            }

            // Get world location using float precision
            FVector StartLocation = GameBoard->GetWorldLocationForTileFloat(StartX, StartY);
            StartLocation.Z = 0.0f; // No Z offset - pieces sit on the board

            PlayerPiece->SetActorLocation(StartLocation, false, nullptr, ETeleportType::TeleportPhysics);

            // Store grid position (use floor for tile occupancy)
            PlayerPiece->GridX = FMath::FloorToInt(StartX);
            PlayerPiece->GridY = FMath::FloorToInt(StartY);

            // Mark the tile as occupied
            AChessTile* StartTile = GameBoard->GetTileAt(PlayerPiece->GridX, PlayerPiece->GridY);
            if (StartTile)
            {
                StartTile->OccupyingPiece = PlayerPiece;
            }

            AllPieces.Add(PlayerPiece);

            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
                    FString::Printf(TEXT("Player spawned at tile (%d, %d), precise pos (%.2f, %.2f)"),
                        PlayerPiece->GridX, PlayerPiece->GridY, StartX, StartY));
            }
        }
    }

    // Spawn enemies and power-ups
    SpawnRandomEnemies(1);
    SpawnRandomPowerUps(10);

    // Show enemy highlights immediately after spawning
    HighlightAllEnemyAttackRanges();

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

    // Show all enemy attack ranges during player's turn
    HighlightAllEnemyAttackRanges();
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
    // Double-check if enemy turn should be skipped
    if (bSkipEnemyTurn)
    {
        bSkipEnemyTurn = false;

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
                TEXT("Enemy turn forcibly ended!"));
        }

        StartNextTurn();
        return;
    }

    if (!GameBoard || !PlayerPiece)
    {
        StartNextTurn();
        return;
    }

    // Keep enemy highlights visible during enemy turn
    // Don't clear them - they should always be visible

    // Reset enemy index
    CurrentEnemyIndex = 0;

    // Start processing enemy turns one by one
    ProcessEnemyTurn();
}

void ATurnBasedGameMode::ProcessEnemyTurn()
{
    if (!GameBoard || !PlayerPiece)
    {
        StartNextTurn();
        return;
    }

    // Collect all valid enemies that haven't acted
    TArray<AChessPieceBase*> ValidEnemies;
    for (AChessPieceBase* Enemy : AllPieces)
    {
        if (Enemy && Enemy != PlayerPiece && !Enemy->bHasActedThisTurn)
        {
            ValidEnemies.Add(Enemy);
        }
    }

    // If no valid enemies, end enemy turn phase
    if (ValidEnemies.Num() == 0)
    {
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
        return;
    }

    // Pick the BEST enemy to act this turn (closest to player or can attack)
    AChessPieceBase* Enemy = nullptr;
    float BestScore = -1.0f;

    for (AChessPieceBase* Candidate : ValidEnemies)
    {
        if (!Candidate || !PlayerPiece)
        {
            continue;
        }

        // Check if this enemy can attack the player
        TArray<FIntPoint> AttackTiles = Candidate->GetAttackTiles(GameBoard);
        bool bCanAttack = false;
        for (const FIntPoint& Tile : AttackTiles)
        {
            if (Tile.X == PlayerPiece->GridX && Tile.Y == PlayerPiece->GridY)
            {
                bCanAttack = true;
                break;
            }
        }

        // Calculate score: prioritize enemies that can attack (score = 1000), otherwise use inverse distance
        float Score = 0.0f;
        if (bCanAttack)
        {
            Score = 1000.0f; // High priority for enemies that can attack
        }
        else
        {
            // Use inverse distance (closer = higher score)
            float Distance = FVector2D::Distance(
                FVector2D(Candidate->GridX, Candidate->GridY),
                FVector2D(PlayerPiece->GridX, PlayerPiece->GridY)
            );
            Score = 100.0f / (Distance + 1.0f); // +1 to avoid division by zero
        }

        if (Score > BestScore)
        {
            BestScore = Score;
            Enemy = Candidate;
        }
    }

    // Fallback to first enemy if none found (shouldn't happen)
    if (!Enemy && ValidEnemies.Num() > 0)
    {
        Enemy = ValidEnemies[0];
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange,
            FString::Printf(TEXT("Enemy at (%d,%d) is acting..."),
                Enemy->GridX, Enemy->GridY));
    }

    // Enemy highlights are always visible, no need to re-highlight here

    // Check if player is in attack range (can be anywhere in attack range, not just adjacent)
    TArray<FIntPoint> AttackTiles = Enemy->GetAttackTiles(GameBoard);
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
        //// Jump attack - move to player's position and capture (like chess pieces)
        //Enemy->JumpAttackPiece(PlayerPiece->GridX, PlayerPiece->GridY, GameBoard);

        //if (GEngine)
        //{
        //    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red,
        //        TEXT("Enemy jumped and captured you!"));
        //}

        //// Check if player is dead
        //if (PlayerPiece->Health <= 0)
        //{
        //    if (GEngine)
        //    {
        //        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
        //            TEXT("GAME OVER! You were captured!"));
        //    }
        //    // TODO: Handle game over
        //}
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

    // Don't clear highlights - they should always be visible

    // ONE enemy has acted - immediately end enemy turn and start player turn
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
            TEXT("One enemy acted. Your turn!"));
    }

    FTimerHandle DelayTimer;
    GetWorld()->GetTimerManager().SetTimer(DelayTimer, [this]()
        {
            StartNextTurn();
        }, 1.0f, false);
}

void ATurnBasedGameMode::HighlightEnemyAttackRange(AChessPieceBase* Enemy)
{
    if (!Enemy || !GameBoard)
    {
        return;
    }

    ClearEnemyHighlights();

    // Highlight valid moves (movement area)
    TArray<FIntPoint> ValidMoves = Enemy->GetValidMoves(GameBoard);
    for (const FIntPoint& Move : ValidMoves)
    {
        AChessTile* ChessTile = GameBoard->GetTileAt(Move.X, Move.Y);
        if (ChessTile)
        {
            ChessTile->Highlight(false); // Blue/green highlight for movement
            HighlightedEnemyTiles.Add(ChessTile);
        }
    }

    // Highlight attack tiles (red highlight)
    TArray<FIntPoint> AttackTiles = Enemy->GetAttackTiles(GameBoard);
    for (const FIntPoint& Tile : AttackTiles)
    {
        AChessTile* ChessTile = GameBoard->GetTileAt(Tile.X, Tile.Y);
        if (ChessTile)
        {
            ChessTile->Highlight(true); // Red highlight for attacks
            HighlightedEnemyTiles.Add(ChessTile);
        }
    }
}

void ATurnBasedGameMode::HighlightAllEnemyAttackRanges()
{
    if (!GameBoard)
    {
        return;
    }

    ClearEnemyHighlights();

    // Highlight attack ranges for all enemies (full range, not just occupied tiles)
    for (AChessPieceBase* Enemy : AllPieces)
    {
        if (Enemy && Enemy != PlayerPiece)
        {
            // Get full attack range (all tiles that can be attacked, including empty ones)
            TArray<FIntPoint> AttackRangeTiles = Enemy->GetAttackRangeTiles(GameBoard);
            for (const FIntPoint& Tile : AttackRangeTiles)
            {
                AChessTile* ChessTile = GameBoard->GetTileAt(Tile.X, Tile.Y);
                if (ChessTile)
                {
                    // Only add if not already highlighted (to avoid duplicates)
                    if (!HighlightedEnemyTiles.Contains(ChessTile))
                    {
                        ChessTile->Highlight(true); // Red highlight for attacks
                        HighlightedEnemyTiles.Add(ChessTile);
                    }
                }
            }
        }
    }
}

void ATurnBasedGameMode::RefreshEnemyHighlights()
{
    // Refresh highlights to remove dead enemies' attack ranges
    HighlightAllEnemyAttackRanges();
}

void ATurnBasedGameMode::ClearEnemyHighlights()
{
    for (AChessTile* Tile : HighlightedEnemyTiles)
    {
        if (Tile)
        {
            Tile->ResetHighlight();
        }
    }

    HighlightedEnemyTiles.Empty();
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
            // Use the same positioning method as player pieces for consistency
            float SpawnCenterX = RandomX + 0.5f;
            float SpawnCenterY = RandomY + 0.5f;
            FVector SpawnLocation = GameBoard->GetWorldLocationForTileFloat(SpawnCenterX, SpawnCenterY);
            SpawnLocation.Z = 0.0f; // No Z offset - pieces sit on the board
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
            // Use the same positioning method as player pieces for consistency
            float CenterX = RandomX + 0.5f;
            float CenterY = RandomY + 0.5f;
            FVector SpawnLocation = GameBoard->GetWorldLocationForTileFloat(CenterX, CenterY);
            SpawnLocation.Z = 50.0f; // Lower than pieces
            FRotator SpawnRotation = FRotator::ZeroRotator;
            FActorSpawnParameters SpawnParams;

            APowerUp* PowerUp = GetWorld()->SpawnActor<APowerUp>(ClassToSpawn, SpawnLocation, SpawnRotation, SpawnParams);

            if (PowerUp)
            {
                // Randomly assign power-up type (50/50 chance)
                PowerUp->PowerUpType = FMath::RandBool() ? EPowerUpType::ExtraMove : EPowerUpType::SuperMode;

                // Set super mode moves count
                PowerUp->SuperModeMovesCount = 5;

                // Force BeginPlay to update mesh
                PowerUp->BeginPlay();

                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
                        FString::Printf(TEXT("Spawned %s at (%d,%d)"),
                            PowerUp->PowerUpType == EPowerUpType::ExtraMove ? TEXT("Extra Move") : TEXT("Super Mode"),
                            RandomX, RandomY));
                }
            }
        }
        else
        {
            i--;
        }
    }
}