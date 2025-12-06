// PowerUp.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerUp.generated.h"

UENUM(BlueprintType)
enum class EPowerUpType : uint8
{
    ExtraMove,
    SuperMode
};

UCLASS()
class DUNGEONCHESS_API APowerUp : public AActor
{
    GENERATED_BODY()

public:
    APowerUp();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUp")
    EPowerUpType PowerUpType;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* PowerUpMesh;

    // Unique mesh for each power-up type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUp|Visuals")
    UStaticMesh* ExtraMoveMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUp|Visuals")
    UStaticMesh* SuperModeMesh;

    // Materials for each type (optional)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUp|Visuals")
    UMaterialInterface* ExtraMoveMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUp|Visuals")
    UMaterialInterface* SuperModeMaterial;

    // Super mode configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUp|SuperMode")
    int32 SuperModeMovesCount = 5;

    void OnPickup(class AChessPieceBase* Piece);
    
    virtual void BeginPlay() override;
protected:
    

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};