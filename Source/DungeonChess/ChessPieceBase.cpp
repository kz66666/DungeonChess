#include "ChessPieceBase.h"
#include "ChessBoard.h"
#include "ChessTile.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

AChessPieceBase::AChessPieceBase()
{
    PrimaryActorTick.bCanEverTick = false;

    // Disable character movement
    GetCharacterMovement()->SetMovementMode(MOVE_None);
    GetCharacterMovement()->bOrientRotationToMovement = false;

    // Set up capsule collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

    GridX = 0;
    GridY = 0;
    bHasActedThisTurn = false;

    Health = 100;
    AttackPower = 10;
    MovementRange = 1;
    PieceType = EPieceType::PlayerPawn;
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

    // Base implementation: can attack adjacent tiles
    TArray<FIntPoint> Directions = {
        FIntPoint(1, 0), FIntPoint(-1, 0),
        FIntPoint(0, 1), FIntPoint(0, -1)
    };

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

    // Move actor to new location
    FVector NewLocation = Board->GetWorldLocationForTile(TargetX, TargetY);
    NewLocation.Z = GetActorLocation().Z; // Maintain Z height
    SetActorLocation(NewLocation);

    bHasActedThisTurn = true;
}

void AChessPieceBase::AttackPiece(AChessPieceBase* Target)
{
    if (!Target)
    {
        return;
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