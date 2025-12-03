// PlayerChessPiece.cpp
#include "PlayerChessPiece.h"
#include "ChessBoard.h"
#include "ChessTile.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

APlayerChessPiece::APlayerChessPiece()
{
    // Create camera boom
    USpringArmComponent* CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 1000.0f; // Further back
    CameraBoom->SetRelativeRotation(FRotator(-30.0f, 0.0f, 0.0f)); // Steeper angle (more top-down)
    CameraBoom->bDoCollisionTest = false;
    CameraBoom->bInheritPitch = false;
    CameraBoom->bInheritRoll = false;
    CameraBoom->bInheritYaw = false; // Don't rotate with character
    CameraBoom->bUsePawnControlRotation = false;

    // Create camera
    UCameraComponent* FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    PieceType = EPieceType::PlayerPawn;
    MovementRange = 1;
}

TArray<FIntPoint> APlayerChessPiece::GetValidMoves(AChessBoard* Board)
{
    TArray<FIntPoint> ValidMoves;

    if (!Board)
    {
        return ValidMoves;
    }

    // Player can move in all 8 directions (including diagonals) but not attack with movement
    TArray<FIntPoint> Directions = {
        FIntPoint(1, 0),   // Right
        FIntPoint(-1, 0),  // Left
        FIntPoint(0, 1),   // Forward
        FIntPoint(0, -1)   // Back
    };

    for (const FIntPoint& Dir : Directions)
    {
        int32 CheckX = GridX + Dir.X;
        int32 CheckY = GridY + Dir.Y;

        if (Board->IsValidPosition(CheckX, CheckY))
        {
            AChessTile* Tile = Board->GetTileAt(CheckX, CheckY);
            if (Tile && !Tile->OccupyingPiece)
            {
                ValidMoves.Add(FIntPoint(CheckX, CheckY));
            }
        }
    }

    return ValidMoves;
}

TArray<FIntPoint> APlayerChessPiece::GetAttackTiles(AChessBoard* Board)
{
    TArray<FIntPoint> AttackTiles;

    if (!Board)
    {
        return AttackTiles;
    }

    // Check all 4 diagonal directions for enemies
    TArray<FIntPoint> Diagonals = {
        FIntPoint(1, 1), FIntPoint(1, -1),
        FIntPoint(-1, 1), FIntPoint(-1, -1)
    };

    for (const FIntPoint& Dir : Diagonals)
    {
        int32 CheckX = GridX + Dir.X;
        int32 CheckY = GridY + Dir.Y;

        if (CanAttackDiagonal(CheckX, CheckY))
        {
            AttackTiles.Add(FIntPoint(CheckX, CheckY));
        }
    }

    return AttackTiles;
}

bool APlayerChessPiece::CanAttackDiagonal(int32 TargetX, int32 TargetY)
{
    // Check if the target is on a diagonal from the player
    int32 DiffX = FMath::Abs(TargetX - GridX);
    int32 DiffY = FMath::Abs(TargetY - GridY);

    return (DiffX == DiffY && DiffX == 1);
}