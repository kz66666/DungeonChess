// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChessPieceBase.h"
#include "BishopChessPiece.generated.h"

/**
 * 
 */
UCLASS()
class DUNGEONCHESS_API ABishopChessPiece : public AChessPieceBase
{
	GENERATED_BODY()

public:
	ABishopChessPiece();

	// Override to implement rook-specific movement
	virtual TArray<FIntPoint> GetValidMoves(class AChessBoard* Board) override;
	
};
