// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ChessPieceBase.generated.h"

UENUM(BlueprintType)
enum class EPieceType : uint8
{
	PlayerPawn,
	EnemyRook,
	EnemyKnight,
	EnemyBishop
};

UCLASS()
class DUNGEONCHESS_API AChessPieceBase : public ACharacter
{
	GENERATED_BODY()

public:
    AChessPieceBase();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Piece")
    EPieceType PieceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 Health = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 AttackPower = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    int32 MovementRange = 1;

    // Current grid position
    int32 GridX;
    int32 GridY;

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* PieceMesh;

    // Turn management
    UPROPERTY()
    bool bHasActedThisTurn = false;

    // Virtual functions for different piece behaviors
    virtual TArray<FIntPoint> GetValidMoves(class AChessBoard* Board);
    virtual TArray<FIntPoint> GetAttackTiles(class AChessBoard* Board);

    // Actions
    virtual void MoveToPiece(int32 TargetX, int32 TargetY, class AChessBoard* Board);
    virtual void AttackPiece(class AChessPieceBase* Target);
    virtual void OnTurnStart();
    virtual void OnTurnEnd();

    // Stealing power mechanic
    void StealPower(AChessPieceBase* Target);
};