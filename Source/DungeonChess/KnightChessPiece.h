// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChessPieceBase.h"
#include "KnightChessPiece.generated.h"

/**
 * 
 */
UCLASS()
class DUNGEONCHESS_API AKnightChessPiece : public AChessPieceBase
{
	GENERATED_BODY()

public:
	AKnightChessPiece();

	// Override to implement knight-specific movement
	virtual TArray<FIntPoint> GetValidMoves(class AChessBoard* Board) override;

	// Override to implement knight-specific attack range (same as movement - L-shapes)
	virtual TArray<FIntPoint> GetAttackTiles(class AChessBoard* Board) override;
	
	// Override to show full attack range (all L-shaped positions)
	virtual TArray<FIntPoint> GetAttackRangeTiles(class AChessBoard* Board) override;

	// Override tick to customize movement animation
	virtual void Tick(float DeltaTime) override;

protected:
	// Calculate the L-shaped path for knight movement
	FVector CalculateKnightMovementPath(float Alpha);

};
