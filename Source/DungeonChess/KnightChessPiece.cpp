// Fill out your copyright notice in the Description page of Project Settings.


#include "KnightChessPiece.h"
#include "ChessBoard.h"
#include "ChessTile.h"

AKnightChessPiece::AKnightChessPiece()
{
    PieceType = EPieceType::EnemyKnight;

    // Make knight move slower than default (default is 800 in base class)
    MoveSpeed = 400.0f;
}

TArray<FIntPoint> AKnightChessPiece::GetValidMoves(AChessBoard* Board)
{
    TArray<FIntPoint> ValidMoves;

    if (!Board)
        return ValidMoves;

    // All 8 possible L-shaped moves
    TArray<FIntPoint> Moves = {
        FIntPoint(2,  1),
        FIntPoint(1,  2),
        FIntPoint(-1,  2),
        FIntPoint(-2,  1),
        FIntPoint(-2, -1),
        FIntPoint(-1, -2),
        FIntPoint(1, -2),
        FIntPoint(2, -1)
    };

    for (const FIntPoint& Move : Moves)
    {
        int32 CheckX = GridX + Move.X;
        int32 CheckY = GridY + Move.Y;

        if (!Board->IsValidPosition(CheckX, CheckY))
            continue;

        AChessTile* Tile = Board->GetTileAt(CheckX, CheckY);
        if (Tile && !Tile->OccupyingPiece)
        {
            ValidMoves.Add(FIntPoint(CheckX, CheckY));
        }
    }

    return ValidMoves;
}

TArray<FIntPoint> AKnightChessPiece::GetAttackTiles(AChessBoard* Board)
{
    TArray<FIntPoint> AttackTiles;

    if (!Board)
    {
        return AttackTiles;
    }

    // Knight attacks in L-shapes (same as movement)
    TArray<FIntPoint> Moves = {
        FIntPoint(2,  1),
        FIntPoint(1,  2),
        FIntPoint(-1,  2),
        FIntPoint(-2,  1),
        FIntPoint(-2, -1),
        FIntPoint(-1, -2),
        FIntPoint(1, -2),
        FIntPoint(2, -1)
    };

    for (const FIntPoint& Move : Moves)
    {
        int32 CheckX = GridX + Move.X;
        int32 CheckY = GridY + Move.Y;

        if (!Board->IsValidPosition(CheckX, CheckY))
            continue;

        AChessTile* Tile = Board->GetTileAt(CheckX, CheckY);
        if (Tile && Tile->OccupyingPiece && !IsAlly(Tile->OccupyingPiece))
        {
            AttackTiles.Add(FIntPoint(CheckX, CheckY));
        }
    }

    return AttackTiles;
}

TArray<FIntPoint> AKnightChessPiece::GetAttackRangeTiles(AChessBoard* Board)
{
    TArray<FIntPoint> RangeTiles;

    if (!Board)
    {
        return RangeTiles;
    }

    // Knight can attack all L-shaped positions (all tiles, not just occupied ones)
    TArray<FIntPoint> Moves = {
        FIntPoint(2,  1),
        FIntPoint(1,  2),
        FIntPoint(-1,  2),
        FIntPoint(-2,  1),
        FIntPoint(-2, -1),
        FIntPoint(-1, -2),
        FIntPoint(1, -2),
        FIntPoint(2, -1)
    };

    for (const FIntPoint& Move : Moves)
    {
        int32 CheckX = GridX + Move.X;
        int32 CheckY = GridY + Move.Y;

        if (Board->IsValidPosition(CheckX, CheckY))
        {
            // Add all valid L-shaped positions (knight can jump, so no blocking check needed)
            RangeTiles.Add(FIntPoint(CheckX, CheckY));
        }
    }

    return RangeTiles;
}

void AKnightChessPiece::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Override movement behavior for knights only
    if (bIsMoving)
    {
        float Distance = FVector::Dist(StartLocation, TargetLocation);

        if (Distance > 0.1f)
        {
            float MoveIncrement = (MoveSpeed * DeltaTime) / Distance;
            MoveAlpha += MoveIncrement;

            if (MoveAlpha >= 1.0f)
            {
                MoveAlpha = 1.0f;
                bIsMoving = false;
                SetActorLocation(TargetLocation);
                OnMovementComplete();
            }
            else
            {
                // Knight-specific L-shaped movement with hop
                float EasedAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, MoveAlpha, 2.0f);
                FVector NewLocation = CalculateKnightMovementPath(EasedAlpha);
                SetActorLocation(NewLocation);
            }
        }
        else
        {
            bIsMoving = false;
            SetActorLocation(TargetLocation);
            OnMovementComplete();
        }
    }
}

FVector AKnightChessPiece::CalculateKnightMovementPath(float Alpha)
{
    // L-shaped movement: Move along one axis first, then the other
    FVector DeltaMove = TargetLocation - StartLocation;
    float AbsX = FMath::Abs(DeltaMove.X);
    float AbsY = FMath::Abs(DeltaMove.Y);

    FVector NewLocation;

    // Determine which axis has more movement (the "2" in the L-shape)
    if (AbsX > AbsY)
    {
        // Move X first (the longer part), then Y
        float XProgress = FMath::Min(Alpha * 1.5f, 1.0f); // X completes at 66%
        float YProgress = FMath::Max((Alpha - 0.5f) * 2.0f, 0.0f); // Y starts at 50%

        NewLocation.X = FMath::Lerp(StartLocation.X, TargetLocation.X, XProgress);
        NewLocation.Y = FMath::Lerp(StartLocation.Y, TargetLocation.Y, YProgress);
    }
    else
    {
        // Move Y first (the longer part), then X
        float YProgress = FMath::Min(Alpha * 1.5f, 1.0f);
        float XProgress = FMath::Max((Alpha - 0.5f) * 2.0f, 0.0f);

        NewLocation.X = FMath::Lerp(StartLocation.X, TargetLocation.X, XProgress);
        NewLocation.Y = FMath::Lerp(StartLocation.Y, TargetLocation.Y, YProgress);
    }

    NewLocation.Z = TargetLocation.Z;

    // Add a hop/arc for the knight's jump
    float HopHeight = 150.0f; // How high the knight jumps
    float HopProgress = FMath::Sin(Alpha * PI); // Creates smooth arc
    NewLocation.Z += HopProgress * HopHeight;

    return NewLocation;
}


