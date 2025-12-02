// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChessPieceBase.h"
#include "RookChessPiece.generated.h"

/**
 * 
 */
UCLASS()
class DUNGEONCHESS_API ARookChessPiece : public AChessPieceBase
{
	GENERATED_BODY()

public:
	ARookChessPiece();

	// Override to implement rook-specific movement
	virtual TArray<FIntPoint> GetValidMoves(class AChessBoard* Board) override;
	
};
