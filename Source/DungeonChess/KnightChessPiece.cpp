// Fill out your copyright notice in the Description page of Project Settings.


#include "KnightChessPiece.h"
#include "ChessBoard.h"
#include "ChessTile.h"

AKnightChessPiece::AKnightChessPiece()
{
    PieceType = EPieceType::EnemyKnight;
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

