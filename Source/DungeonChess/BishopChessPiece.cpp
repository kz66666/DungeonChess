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



