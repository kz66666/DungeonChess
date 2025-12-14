#include "ChessPieceBase.h"
#include "ChessBoard.h"
#include "ChessTile.h"
#include "PowerUp.h"
#include "TurnBasedGameMode.h"
#include "PlayerChessPiece.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Engine/OverlapResult.h"

AChessPieceBase::AChessPieceBase()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    // Disable character movement
    GetCharacterMovement()->SetMovementMode(MOVE_None);
    GetCharacterMovement()->GravityScale = 0.0f;
    GetCharacterMovement()->bOrientRotationToMovement = false;

    // Set up capsule collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    GetCapsuleComponent()->SetCollisionObjectType(ECC_Pawn);
    GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);

    GridX = 0;
    GridY = 0;
    bHasActedThisTurn = false;

    Health = 100;
    AttackPower = 25;
    MovementRange = 1;
    PieceType = EPieceType::PlayerPawn;

    // Movement animation variables
    bIsMoving = false;
    MoveSpeed = 800.0f; // Units per second (increased for visibility)
    MoveAlpha = 0.0f;

    PieceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PieceMesh"));
    if (PieceMesh)
    {
        PieceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    RootComponent = PieceMesh;

}

void AChessPieceBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsMoving)
    {
        // Calculate interpolation progress
        float Distance = FVector::Dist(StartLocation, TargetLocation);

        if (Distance > 0.1f)
        {
            float MoveIncrement = (MoveSpeed * DeltaTime) / Distance;
            MoveAlpha += MoveIncrement;

            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White,
                    FString::Printf(TEXT("Moving: Alpha=%.2f, DeltaTime=%.3f"), MoveAlpha, DeltaTime));
            }

            if (MoveAlpha >= 1.0f)
            {
                // Movement complete
                MoveAlpha = 1.0f;
                bIsMoving = false;
                SetActorLocation(TargetLocation);
                OnMovementComplete();
            }
            else
            {
                // Interpolate position (smooth ease in/out)
                float EasedAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, MoveAlpha, 2.0f);
                FVector NewLocation = FMath::Lerp(StartLocation, TargetLocation, EasedAlpha);
                SetActorLocation(NewLocation);
            }
        }
        else
        {
            // Already at target
            bIsMoving = false;
            SetActorLocation(TargetLocation);
            OnMovementComplete();
        }
    }
}

TArray<FIntPoint> AChessPieceBase::GetValidMoves(AChessBoard* Board)
{
    TArray<FIntPoint> ValidMoves;

    if (!Board)
    {
        return ValidMoves;
    }

    // Default: can move 1 tile in any cardinal direction
    TArray<FIntPoint> Directions = {
        FIntPoint(1, 0), FIntPoint(-1, 0),
        FIntPoint(0, 1), FIntPoint(0, -1)
    };

    for (const FIntPoint& Dir : Directions)
    {
        for (int32 i = 1; i <= MovementRange; i++)
        {
            int32 CheckX = GridX + (Dir.X * i);
            int32 CheckY = GridY + (Dir.Y * i);

            if (Board->IsValidPosition(CheckX, CheckY))
            {
                AChessTile* Tile = Board->GetTileAt(CheckX, CheckY);
                if (Tile && !Tile->OccupyingPiece)
                {
                    ValidMoves.Add(FIntPoint(CheckX, CheckY));
                }
                else
                {
                    // Blocked by another piece
                    break;
                }
            }
        }
    }

    return ValidMoves;
}

TArray<FIntPoint> AChessPieceBase::GetAttackTiles(AChessBoard* Board)
{
    TArray<FIntPoint> AttackTiles;

    if (!Board)
    {
        return AttackTiles;
    }

    // Choose directions based on super mode
    TArray<FIntPoint> Directions;
    if (bSuperModeActive)
    {
        // All 8 directions in super mode
        Directions = {
            FIntPoint(1, 0), FIntPoint(-1, 0),
            FIntPoint(0, 1), FIntPoint(0, -1),
            FIntPoint(1, 1), FIntPoint(1, -1),
            FIntPoint(-1, 1), FIntPoint(-1, -1)
        };
    }
    else
    {
        // Normal - only 4 cardinal directions
        Directions = {
            FIntPoint(1, 0), FIntPoint(-1, 0),
            FIntPoint(0, 1), FIntPoint(0, -1)
        };
    }

    for (const FIntPoint& Dir : Directions)
    {
        int32 CheckX = GridX + Dir.X;
        int32 CheckY = GridY + Dir.Y;

        if (Board->IsValidPosition(CheckX, CheckY))
        {
            AChessTile* Tile = Board->GetTileAt(CheckX, CheckY);
            if (Tile && Tile->OccupyingPiece && Tile->OccupyingPiece != this)
            {
                // Only attack enemies, not allies
                if (!IsAlly(Tile->OccupyingPiece))
                {
                    AttackTiles.Add(FIntPoint(CheckX, CheckY));
                }
            }
        }
    }

    return AttackTiles;
}

TArray<FIntPoint> AChessPieceBase::GetAttackRangeTiles(AChessBoard* Board)
{
    // Default implementation: return attack tiles (occupied only)
    // Override in subclasses to show full range
    return GetAttackTiles(Board);
}

void AChessPieceBase::MoveToPiece(int32 TargetX, int32 TargetY, AChessBoard* Board)
{
    if (!Board)
    {
        return;
    }

    if (bIsMoving)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, TEXT("Already moving!"));
        }
        return;
    }

    // Get old and new tiles
    AChessTile* OldTile = Board->GetTileAt(GridX, GridY);
    AChessTile* NewTile = Board->GetTileAt(TargetX, TargetY);

    if (!NewTile || NewTile->OccupyingPiece)
    {
        return;
    }

    // Update tile references
    if (OldTile)
    {
        OldTile->OccupyingPiece = nullptr;
    }

    NewTile->OccupyingPiece = this;

    // Check for power-ups on the target tile and collect them
    // Use multiple methods to ensure we find power-ups
    FVector TileLocation = NewTile->GetActorLocation();
    
    // Method 1: Overlap query by object type
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredActor(NewTile);
    GetWorld()->OverlapMultiByObjectType(
        OverlapResults,
        TileLocation,
        FQuat::Identity,
        FCollisionObjectQueryParams(ECC_WorldDynamic),
        FCollisionShape::MakeSphere(100.0f), // Larger sphere to catch power-ups
        QueryParams
    );
    
    // Method 2: Also search by class directly (more reliable)
    TArray<AActor*> FoundPowerUps;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APowerUp::StaticClass(), FoundPowerUps);
    
    // Check both overlap results and found actors
    for (const FOverlapResult& Result : OverlapResults)
    {
        APowerUp* PowerUp = Cast<APowerUp>(Result.GetActor());
        if (PowerUp && IsValid(PowerUp))
        {
            float Distance = FVector::Dist(TileLocation, PowerUp->GetActorLocation());
            if (Distance < 100.0f) // Within range
            {
                PowerUp->OnPickup(this);
                break; // Only collect one power-up per move
            }
        }
    }
    
    // If not found via overlap, check by distance from found actors
    if (OverlapResults.Num() == 0 || !Cast<APowerUp>(OverlapResults[0].GetActor()))
    {
        for (AActor* Actor : FoundPowerUps)
        {
            APowerUp* PowerUp = Cast<APowerUp>(Actor);
            if (PowerUp && IsValid(PowerUp))
            {
                float Distance = FVector::Dist(TileLocation, PowerUp->GetActorLocation());
                if (Distance < 100.0f) // Within range
                {
                    PowerUp->OnPickup(this);
                    break; // Only collect one power-up per move
                }
            }
        }
    }

    // Update grid position
    GridX = TargetX;
    GridY = TargetY;

    // Set up smooth movement
    StartLocation = GetActorLocation();
    // Reuse TileLocation from power-up check above
    TargetLocation = TileLocation + FVector(25.0f, 50.0f, 0.0f); // No Z offset - pieces sit on the board 

    MoveAlpha = 0.0f;
    bIsMoving = true;

    if (MoveSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, MoveSound, StartLocation);
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan,
            FString::Printf(TEXT("Starting move to (%d, %d) | From: (%.1f, %.1f, %.1f) To: (%.1f, %.1f, %.1f)"),
                TargetX, TargetY,
                StartLocation.X, StartLocation.Y, StartLocation.Z,
                TargetLocation.X, TargetLocation.Y, TargetLocation.Z));
    }
}

void AChessPieceBase::OnMovementComplete()
{
    bHasActedThisTurn = true;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green,
            FString::Printf(TEXT("Arrived at (%d, %d)"), GridX, GridY));
    }
}

void AChessPieceBase::ActivateSuperMode(int32 Moves)
{
    bSuperModeActive = true;
    SuperModeMovesRemaining = Moves;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta,
            FString::Printf(TEXT("*** SUPER MODE! %d moves ***"), Moves));
    }
}

void AChessPieceBase::DeactivateSuperMode()
{
    bSuperModeActive = false;
    SuperModeMovesRemaining = 0;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::White, TEXT("Super Mode ended"));
    }
}

void AChessPieceBase::AttackPiece(AChessPieceBase* Target)
{
    if (!Target)
    {
        return;
    }

    if (ClashSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ClashSound, GetActorLocation());
    }

    // Deal damage
    Target->Health -= AttackPower;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red,
            FString::Printf(TEXT("%s attacked %s for %d damage! Target health: %d"),
                *GetName(), *Target->GetName(), AttackPower, Target->Health));
    }

    // Check if target died
    if (Target->Health <= 0)
    {
        // Check if target is player with revive
        if (APlayerChessPiece* PlayerTarget = Cast<APlayerChessPiece>(Target))
        {
            if (PlayerTarget->RevivesRemaining > 0) 
            {
                // Use one revive!
                PlayerTarget->RevivesRemaining--;  
                PlayerTarget->Health = 100; // Full health restoration

                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
                        FString::Printf(TEXT("REVIVED! %d revives remaining!"),
                            PlayerTarget->RevivesRemaining));
                }

                // Don't destroy the player
                bHasActedThisTurn = true;
                return;
            }
        }

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Yellow,
                FString::Printf(TEXT("%s was defeated!"), *Target->GetName()));
        }

        // Optional: steal power on kill
        StealPower(Target);

        // Remove from tile
        AChessBoard* Board = nullptr;
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AChessBoard::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            Board = Cast<AChessBoard>(FoundActors[0]);
        }

        if (Board)
        {
            AChessTile* Tile = Board->GetTileAt(Target->GridX, Target->GridY);
            if (Tile)
            {
                Tile->OccupyingPiece = nullptr;
            }
        }

        // Notify game mode to refresh highlights and remove from list when enemy dies
        ATurnBasedGameMode* GameMode = Cast<ATurnBasedGameMode>(GetWorld()->GetAuthGameMode());
        if (GameMode && Target != this && Target->PieceType != EPieceType::PlayerPawn)
        {
            // Remove enemy from AllPieces array
            GameMode->AllPieces.Remove(Target);
            // Enemy is being destroyed - refresh highlights
            GameMode->RefreshEnemyHighlights();
        }

        Target->Destroy();
    }

    bHasActedThisTurn = true;
}

void AChessPieceBase::JumpAttackPiece(int32 TargetX, int32 TargetY, AChessBoard* Board)
{
    if (!Board || bIsMoving)
    {
        return;
    }

    AChessTile* TargetTile = Board->GetTileAt(TargetX, TargetY);
    if (!TargetTile || !TargetTile->OccupyingPiece)
    {
        return;
    }

    AChessPieceBase* Target = TargetTile->OccupyingPiece;

    // Don't attack allies
    if (IsAlly(Target))
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange,
                TEXT("Cannot attack ally!"));
        }
        return;
    }

    // Play attack sound
    if (AttackSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, AttackSound, GetActorLocation());
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red,
            FString::Printf(TEXT("Captured %s!"), *Target->GetName()));
    }

    // Get old tile
    AChessTile* OldTile = Board->GetTileAt(GridX, GridY);
    if (OldTile)
    {
        OldTile->OccupyingPiece = nullptr;
    }

    // Steal power and destroy target
    StealPower(Target);
    
    // Notify game mode to refresh highlights and remove from list when enemy dies
    ATurnBasedGameMode* GameMode = Cast<ATurnBasedGameMode>(GetWorld()->GetAuthGameMode());
    if (GameMode && Target != this && Target->PieceType != EPieceType::PlayerPawn)
    {
        // Remove enemy from AllPieces array
        GameMode->AllPieces.Remove(Target);
        // Enemy is being destroyed - refresh highlights
        GameMode->RefreshEnemyHighlights();
    }

    // Check if target is player with revive BEFORE destroying
    bool bTargetRevived = false;
    if (APlayerChessPiece* PlayerTarget = Cast<APlayerChessPiece>(Target))
    {
        if (PlayerTarget->RevivesRemaining > 0)  
        {
            // Use one revive!
            PlayerTarget->RevivesRemaining--; 
            PlayerTarget->Health = 100;
            bTargetRevived = true;

            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
                    FString::Printf(TEXT("*** REVIVED! %d revives remaining! ***"),
                        PlayerTarget->RevivesRemaining));
            }
        }
    }

    if (bTargetRevived)
    {
        // Target survived - don't capture
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange,
                TEXT("Capture failed - target revived!"));
        }

        bHasActedThisTurn = true;
        return;
    }
    
    Target->Destroy();

    // Update grid position
    GridX = TargetX;
    GridY = TargetY;

    // Start smooth movement to target position
    StartLocation = GetActorLocation();
    FVector TileLocation = TargetTile->GetActorLocation();
    TargetLocation = TileLocation + FVector(25.0f, 50.0f, 0.0f); // No Z offset - pieces sit on the board

    MoveAlpha = 0.0f;
    bIsMoving = true;

    TargetTile->OccupyingPiece = this;
    bHasActedThisTurn = true;
}

void AChessPieceBase::StealPower(AChessPieceBase* Target)
{
    if (!Target)
    {
        return;
    }

    // Steal a percentage of the target's power
    int32 StolenPower = FMath::RoundToInt(Target->AttackPower * 0.5f);
    AttackPower += StolenPower;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Purple,
            FString::Printf(TEXT("%s stole %d power! New attack: %d"),
                *GetName(), StolenPower, AttackPower));
    }
}

bool AChessPieceBase::IsAlly(AChessPieceBase* OtherPiece) const
{
    if (!OtherPiece)
    {
        return false;
    }

    // Player pieces are allies to each other, enemy pieces are allies to each other
    bool bThisIsPlayer = (PieceType == EPieceType::PlayerPawn);
    bool bOtherIsPlayer = (OtherPiece->PieceType == EPieceType::PlayerPawn);

    // They are allies if they are both players or both enemies
    return (bThisIsPlayer && bOtherIsPlayer) || (!bThisIsPlayer && !bOtherIsPlayer);
}

void AChessPieceBase::OnTurnStart()
{
    bHasActedThisTurn = false;
}

void AChessPieceBase::OnTurnEnd()
{
    // Override in subclasses if needed
}