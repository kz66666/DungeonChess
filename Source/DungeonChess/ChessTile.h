// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChessTile.generated.h"

UCLASS()
class DUNGEONCHESS_API AChessTile : public AActor
{
	GENERATED_BODY()
	
public:
    AChessTile();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* TileMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* NormalMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* DarkMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* HighlightedMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* AttackHighlightMaterial;

    int32 GridX;
    int32 GridY;

    void Highlight(bool bIsAttackTile = false);
    void ResetHighlight();

    UPROPERTY()
    class AChessPieceBase* OccupyingPiece;

private:
    UPROPERTY()
    UMaterialInterface* OriginalMaterial;
};
