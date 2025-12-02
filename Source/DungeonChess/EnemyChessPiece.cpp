// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyChessPiece.h"
#include "ChessBoard.h"
#include "Kismet/KismetMathLibrary.h"
#include "EngineUtils.h"

// Sets default values
AEnemyChessPiece::AEnemyChessPiece()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Ensures this enemy chess piece gets an AI Controller as soon as it's spawned
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

}

// Called when the game starts or when spawned
void AEnemyChessPiece::BeginPlay()
{
	Super::BeginPlay();

	FindChessBoard();

	// Initialize the world position from BoardPosition
	if (ChessBoard)
	{	
		// Converts board position to world coordinates
		TargetWorldLocation = ChessBoard->GetWorldLocationForTile(BoardPosition.X, BoardPosition.Y);

		SetActorLocation(TargetWorldLocation);
	}
	
}

// Called every frame
void AEnemyChessPiece::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector CurrentLocation = GetActorLocation();
	
	if (!CurrentLocation.Equals(TargetWorldLocation, 1.f))
	{
		FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetWorldLocation, DeltaTime, MoveSpeed);
		SetActorLocation(NewLocation);
	}
	
}

// Called to bind functionality to input
void AEnemyChessPiece::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemyChessPiece::FindChessBoard()
{	
	// Checks if chess board is already found/assigned
	if (ChessBoard) return;

	// Loop through all actors in the level to find the chess board
	for (TActorIterator<AChessBoard> It(GetWorld()); It; ++It)
	{
		ChessBoard = *It;
		return;
	}

	UE_LOG(LogTemp, Error, TEXT("EnemyChessPiece: No ChessBoard found in the level"));
}

void AEnemyChessPiece::MoveToBoardPosition(FVector2D NewBoardPos) {
	if (!ChessBoard) {
		return;
	}

	// Update board coordinate
	BoardPosition = NewBoardPos;

	// Convert to world
	TargetWorldLocation = ChessBoard->GetWorldLocationForTile(BoardPosition.X, BoardPosition.Y);

	// If smooth movement OFF → snap instantly
	if (!bUseSmoothMovement)
	{
		SetActorLocation(TargetWorldLocation);
	}
}



