// TurnBasedGameMode.h
#pragma once

#include "CoreMinimal.h"
#include "ChessPieceBase.h"
#include "GameFramework/GameModeBase.h"
#include "TurnBasedGameMode.generated.h"

UCLASS()
class DUNGEONCHESS_API ATurnBasedGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ATurnBasedGameMode();

    /*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Classes")
    TSubclassOf<class AChessPieceBase> EnemyPieceClass;*/

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Classes")
    TArray<TSubclassOf<AChessPieceBase>> EnemyPieceClasses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Classes")
    TSubclassOf<class APowerUp> PowerUpClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Spawn")
    bool bRandomPlayerSpawn = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Spawn", meta = (EditCondition = "!bRandomPlayerSpawn"))
    float PlayerStartX = 4.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Spawn", meta = (EditCondition = "!bRandomPlayerSpawn"))
    float PlayerStartY = 4.5f;

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

    // Called when player completes their action
    void OnPlayerAction();

    void StartNextTurn();
    void ExecuteEnemyTurns();

    void SpawnRandomEnemies(int32 Count);
    void SpawnRandomPowerUps(int32 Count);
    
    // Refresh enemy highlights (e.g., when enemies die)
    void RefreshEnemyHighlights();

    UPROPERTY(BlueprintReadWrite, Category = "Turn Management")
    bool bSkipEnemyTurn = false;

protected:
    virtual void BeginPlay() override;

private:
    void InitializeGame();

    // Timer for enemy turn execution
    FTimerHandle EnemyTurnTimerHandle;
    void ProcessEnemyTurn();
    int32 CurrentEnemyIndex = 0;

    void HighlightEnemyAttackRange(class AChessPieceBase* Enemy);
    void HighlightAllEnemyAttackRanges();
    void ClearEnemyHighlights();
    TArray<class AChessTile*> HighlightedEnemyTiles;
};