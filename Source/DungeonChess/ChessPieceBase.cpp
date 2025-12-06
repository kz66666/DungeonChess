#include "ChessPieceBase.h"
#include "ChessBoard.h"
#include "ChessTile.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

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
                AttackTiles.Add(FIntPoint(CheckX, CheckY));
            }
        }
    }

    return AttackTiles;
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

    // Update grid position
    GridX = TargetX;
    GridY = TargetY;

    // Set up smooth movement
    StartLocation = GetActorLocation();
    //TargetLocation = NewTile->GetActorLocation();
    FVector TileLocation = NewTile->GetActorLocation();
    TargetLocation = TileLocation + FVector(25.0f, 50.0f, 100.0f);
    //TargetLocation.Z = 100.0f; 

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
    Target->Destroy();

    // Update grid position
    GridX = TargetX;
    GridY = TargetY;

    // Start smooth movement to target position
    StartLocation = GetActorLocation();
    FVector TileLocation = TargetTile->GetActorLocation();
    TargetLocation = TileLocation + FVector(25.0f, 50.0f, 100.0f);

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

void AChessPieceBase::OnTurnStart()
{
    bHasActedThisTurn = false;
}

void AChessPieceBase::OnTurnEnd()
{
    // Override in subclasses if needed
}