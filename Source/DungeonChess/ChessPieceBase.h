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
	EnemyBishop,
	EnemyQueen,
};

UCLASS()
class DUNGEONCHESS_API AChessPieceBase : public ACharacter
{
	GENERATED_BODY()

protected:
    // Movement animation variables
    UPROPERTY()
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MoveSpeed; // Units per second

    UPROPERTY()
    FVector StartLocation;

    UPROPERTY()
    FVector TargetLocation;

    UPROPERTY()
    float MoveAlpha;

    // Called when smooth movement completes
    virtual void OnMovementComplete();

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* MoveSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* AttackSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* ClashSound; // Only for player

    // Current grid position
    int32 GridX;
    int32 GridY;

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* PieceMesh;

    // Turn management
    UPROPERTY()
    bool bHasActedThisTurn = false;

    UPROPERTY(BlueprintReadWrite, Category = "PowerUp")
    bool bSuperModeActive = false;

    UPROPERTY(BlueprintReadWrite, Category = "PowerUp")
    int32 SuperModeMovesRemaining = 0;

    void ActivateSuperMode(int32 Moves);
    void DeactivateSuperMode();

    // Virtual functions for different piece behaviors
    virtual TArray<FIntPoint> GetValidMoves(class AChessBoard* Board);
    virtual TArray<FIntPoint> GetAttackTiles(class AChessBoard* Board);
    
    // Get all tiles in attack range (for highlighting - includes empty tiles)
    virtual TArray<FIntPoint> GetAttackRangeTiles(class AChessBoard* Board);

    // Actions
    virtual void MoveToPiece(int32 TargetX, int32 TargetY, class AChessBoard* Board);
    virtual void AttackPiece(class AChessPieceBase* Target);
    virtual void JumpAttackPiece(int32 TargetX, int32 TargetY, class AChessBoard* Board);
    virtual void OnTurnStart();
    virtual void OnTurnEnd();

    // Stealing power mechanic
    void StealPower(AChessPieceBase* Target);

    // Helper function to check if a piece is an ally (same team)
    bool IsAlly(AChessPieceBase* OtherPiece) const;

    // Override Tick function
    virtual void Tick(float DeltaTime) override;
};