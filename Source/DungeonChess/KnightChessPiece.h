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

	// Override to implement rook-specific movement
	virtual TArray<FIntPoint> GetValidMoves(class AChessBoard* Board) override;

	// Override tick to customize movement animation
	virtual void Tick(float DeltaTime) override;

protected:
	// Calculate the L-shaped path for knight movement
	FVector CalculateKnightMovementPath(float Alpha);

};
