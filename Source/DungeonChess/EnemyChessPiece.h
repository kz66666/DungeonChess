// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "EnemyChessPiece.generated.h"

class AChessBoard;

UCLASS()
class DUNGEONCHESS_API AEnemyChessPiece : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AEnemyChessPiece();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// The board this chess piece belongs to
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chess")
	AChessBoard* ChessBoard;

	// The tile the enemy chess piece is on
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess")
	FVector2D BoardPosition;

	// Moves the chess piece to a new board coordinate 
	UFUNCTION(BlueprintCallable, Category = "Chess")
	void MoveToBoardPosition(FVector2D NewBoardPos);

	// So the chess piece slides smoothly from tile to tile instead of instant teleportaion
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess")
	bool bUseSmoothMovement = true;

	// The speed at which the enemy chess piece moves
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess")
	float MoveSpeed = 300.f;
	
	// Return the list of valid moves (override in subclasses)
	UFUNCTION(BlueprintCallable, Category = "Chess")
	virtual TArray<FVector2D> GetValidMoves() const;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* PieceMesh;

private:
	/** Internal target world location when moving */
	FVector TargetWorldLocation;

	/** Finds the ChessBoard actor in the level if not assigned */
	void FindChessBoard();
};
