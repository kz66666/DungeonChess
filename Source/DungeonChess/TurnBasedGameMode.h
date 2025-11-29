// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TurnBasedGameMode.generated.h"

/**
 * 
 */
UCLASS()
class DUNGEONCHESS_API ATurnBasedGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
    ATurnBasedGameMode();

    UPROPERTY()
    class AChessBoard* GameBoard;

    UPROPERTY()
    TArray<class AChessPieceBase*> AllPieces;

    UPROPERTY()
    class APlayerChessPiece* PlayerPiece;

    UPROPERTY()
    int32 CurrentTurn = 0;

    UPROPERTY()
    bool bPlayerTurn = true;

    void StartNextTurn();
    void EndPlayerTurn();
    void ExecuteEnemyTurns();

    void SpawnRandomEnemies(int32 Count);
    void SpawnRandomPowerUps(int32 Count);

protected:
    virtual void BeginPlay() override;

private: 
    void InitializeGame();
};
