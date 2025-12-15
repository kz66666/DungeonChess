// Fill out your copyright notice in the Description page of Project Settings.


#include "QueenChessPiece.h"
#include "ChessBoard.h"
#include "ChessTile.h"

AQueenChessPiece::AQueenChessPiece()
{
    PieceType = EPieceType::EnemyQueen;
}

TArray<FIntPoint> AQueenChessPiece::GetValidMoves(AChessBoard* Board)
{
    TArray<FIntPoint> ValidMoves;
    if (!Board)
        return ValidMoves;

    static const int32 QueenMaxRange = 4;

    TArray<FIntPoint> Directions = {
        // Rook
        FIntPoint(1, 0), FIntPoint(-1, 0),
        FIntPoint(0, 1), FIntPoint(0, -1),
        // Bishop
        FIntPoint(1, 1), FIntPoint(-1, -1),
        FIntPoint(1, -1), FIntPoint(-1, 1)
    };

    for (const FIntPoint& Dir : Directions)
    {
        for (int32 i = 1; i <= QueenMaxRange; i++)
        {
            int32 CheckX = GridX + Dir.X * i;
            int32 CheckY = GridY + Dir.Y * i;

            if (!Board->IsValidPosition(CheckX, CheckY))
                break;

            AChessTile* Tile = Board->GetTileAt(CheckX, CheckY);
            if (!Tile || Tile->OccupyingPiece)
                break;

            ValidMoves.Add(FIntPoint(CheckX, CheckY));
        }
    }

    return ValidMoves;
}

TArray<FIntPoint> AQueenChessPiece::GetAttackTiles(AChessBoard* Board)
{
    TArray<FIntPoint> AttackTiles;
    if (!Board)
        return AttackTiles;

    static const int32 QueenMaxRange = 3;

    TArray<FIntPoint> Directions = {
        FIntPoint(1, 0), FIntPoint(-1, 0),
        FIntPoint(0, 1), FIntPoint(0, -1),
        FIntPoint(1, 1), FIntPoint(-1, -1),
        FIntPoint(1, -1), FIntPoint(-1, 1)
    };

    for (const FIntPoint& Dir : Directions)
    {
        for (int32 i = 1; i <= QueenMaxRange; i++)
        {
            int32 CheckX = GridX + Dir.X * i;
            int32 CheckY = GridY + Dir.Y * i;

            if (!Board->IsValidPosition(CheckX, CheckY))
                break;

            AChessTile* Tile = Board->GetTileAt(CheckX, CheckY);
            if (!Tile)
                break;

            if (Tile->OccupyingPiece)
            {
                if (!IsAlly(Tile->OccupyingPiece))
                {
                    AttackTiles.Add(FIntPoint(CheckX, CheckY));
                }
                break;
            }
        }
    }

    return AttackTiles;
}


TArray<FIntPoint> AQueenChessPiece::GetAttackRangeTiles(AChessBoard* Board)
{
    TArray<FIntPoint> RangeTiles;
    if (!Board)
        return RangeTiles;

    static const int32 QueenMaxRange = 4;

    TArray<FIntPoint> Directions = {
        FIntPoint(1, 0), FIntPoint(-1, 0),
        FIntPoint(0, 1), FIntPoint(0, -1),
        FIntPoint(1, 1), FIntPoint(-1, -1),
        FIntPoint(1, -1), FIntPoint(-1, 1)
    };

    for (const FIntPoint& Dir : Directions)
    {
        for (int32 i = 1; i <= QueenMaxRange; i++)
        {
            int32 CheckX = GridX + Dir.X * i;
            int32 CheckY = GridY + Dir.Y * i;

            if (!Board->IsValidPosition(CheckX, CheckY))
                break;

            RangeTiles.Add(FIntPoint(CheckX, CheckY));

            AChessTile* Tile = Board->GetTileAt(CheckX, CheckY);
            if (Tile && Tile->OccupyingPiece)
                break;
        }
    }

    return RangeTiles;
}
