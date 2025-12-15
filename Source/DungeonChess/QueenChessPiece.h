// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChessPieceBase.h"
#include "QueenChessPiece.generated.h"

/**
 * 
 */
UCLASS()
class DUNGEONCHESS_API AQueenChessPiece : public AChessPieceBase
{
	GENERATED_BODY()

public:
	AQueenChessPiece();

	// Override to implement rook-specific movement
	virtual TArray<FIntPoint> GetValidMoves(class AChessBoard* Board) override;

	// Override to implement rook-specific attack range (same as movement)
	virtual TArray<FIntPoint> GetAttackTiles(class AChessBoard* Board) override;

	// Override to show full attack range (entire row and column)
	virtual TArray<FIntPoint> GetAttackRangeTiles(class AChessBoard* Board) override;
	
};
