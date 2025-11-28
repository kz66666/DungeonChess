// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChessBoard.generated.h"

UCLASS()
class DUNGEONCHESS_API AChessBoard : public AActor
{
	GENERATED_BODY()
	
public:
    AChessBoard();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Board")
    int32 BoardWidth = 16;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Board")
    int32 BoardHeight = 16;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Board")
    float TileSize = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Board")
    TSubclassOf<class AChessTile> TileClass;


protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TArray<class AChessTile*> Tiles;

    void GenerateBoard();

public:
    AChessTile* GetTileAt(int32 X, int32 Y);
    FVector GetWorldLocationForTile(int32 X, int32 Y);
    bool IsValidPosition(int32 X, int32 Y);
};
