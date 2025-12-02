// Fill out your copyright notice in the Description page of Project Settings.

#include "RookChessPiece.h"
#include "ChessBoard.h"
#include "ChessTile.h"

ARookChessPiece::ARookChessPiece()
{
    PieceType = EPieceType::EnemyRook;
}

TArray<FIntPoint> ARookChessPiece::GetValidMoves(class AChessBoard* Board) {
    TArray<FIntPoint> ValidMoves;

    if (!Board)
    {
        return ValidMoves;
    }

    // Rook moves horizontally and vertically
    TArray<FIntPoint> Directions = {
        FIntPoint(1, 0), FIntPoint(-1, 0),
        FIntPoint(0, 1), FIntPoint(0, -1)
    };

    for (const FIntPoint& Dir : Directions)
    {   
        int32 MaxSteps = (Dir.X != 0) ? Board->BoardWidth : Board->BoardHeight;
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


