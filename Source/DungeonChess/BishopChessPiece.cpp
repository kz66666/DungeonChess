// Fill out your copyright notice in the Description page of Project Settings.


#include "BishopChessPiece.h"
#include "ChessBoard.h"
#include "ChessTile.h"

ABishopChessPiece::ABishopChessPiece()
{
    PieceType = EPieceType::EnemyBishop;
}

TArray<FIntPoint> ABishopChessPiece::GetValidMoves(class AChessBoard* Board) {
    TArray<FIntPoint> ValidMoves;

    if (!Board)
    {
        return ValidMoves;
    }

    // Rook moves horizontally and vertically
    TArray<FIntPoint> Directions = {
        FIntPoint(1, 1), FIntPoint(-1, -1),
        FIntPoint(1, -1), FIntPoint(-1, 1)
    };

    // Max steps based on board size
    int32 MaxSteps = FMath::Max(Board->BoardWidth, Board->BoardHeight);

    for (const FIntPoint& Dir : Directions)
    {
        for (int32 i = 1; i <= MaxSteps; i++)
        {
            int32 CheckX = GridX + (Dir.X * i);
            int32 CheckY = GridY + (Dir.Y * i);

            if (!Board->IsValidPosition(CheckX, CheckY)) break;

            AChessTile* Tile = Board->GetTileAt(CheckX, CheckY);

            if (Tile && !Tile->OccupyingPiece)
            {
                ValidMoves.Add(FIntPoint(CheckX, CheckY));
            }
        }
    }

    return ValidMoves;
}

TArray<FIntPoint> ABishopChessPiece::GetAttackTiles(AChessBoard* Board)
{
    TArray<FIntPoint> AttackTiles;

    if (!Board)
    {
        return AttackTiles;
    }

    // Bishop attacks diagonally (same as movement)
    TArray<FIntPoint> Directions = {
        FIntPoint(1, 1), FIntPoint(-1, -1),
        FIntPoint(1, -1), FIntPoint(-1, 1)
    };

    int32 MaxSteps = FMath::Max(Board->BoardWidth, Board->BoardHeight);

    for (const FIntPoint& Dir : Directions)
    {
        for (int32 i = 1; i <= MaxSteps; i++)
        {
            int32 CheckX = GridX + (Dir.X * i);
            int32 CheckY = GridY + (Dir.Y * i);

            if (!Board->IsValidPosition(CheckX, CheckY)) break;

            AChessTile* Tile = Board->GetTileAt(CheckX, CheckY);
            if (Tile)
            {
                if (Tile->OccupyingPiece && !IsAlly(Tile->OccupyingPiece))
                {
                    // Found an enemy piece - can attack it
                    AttackTiles.Add(FIntPoint(CheckX, CheckY));
                    break; // Can't attack through pieces
                }
                else if (Tile->OccupyingPiece)
                {
                    // Blocked by ally - stop checking this direction
                    break;
                }
            }
        }
    }

    return AttackTiles;
}

TArray<FIntPoint> ABishopChessPiece::GetAttackRangeTiles(AChessBoard* Board)
{
    TArray<FIntPoint> RangeTiles;

    if (!Board)
    {
        return RangeTiles;
    }

    // Bishop can attack entire diagonals (all tiles, not just occupied ones)
    TArray<FIntPoint> Directions = {
        FIntPoint(1, 1), FIntPoint(-1, -1),
        FIntPoint(1, -1), FIntPoint(-1, 1)
    };

    int32 MaxSteps = FMath::Max(Board->BoardWidth, Board->BoardHeight);

    for (const FIntPoint& Dir : Directions)
    {
        for (int32 i = 1; i <= MaxSteps; i++)
        {
            int32 CheckX = GridX + (Dir.X * i);
            int32 CheckY = GridY + (Dir.Y * i);

            if (!Board->IsValidPosition(CheckX, CheckY)) break;

            // Add all tiles in range (including empty ones)
            RangeTiles.Add(FIntPoint(CheckX, CheckY));
            
            // Stop if we hit a piece (can't attack through pieces)
            AChessTile* Tile = Board->GetTileAt(CheckX, CheckY);
            if (Tile && Tile->OccupyingPiece)
            {
                break;
            }
        }
    }

    return RangeTiles;
}

