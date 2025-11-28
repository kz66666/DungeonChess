// PowerUp.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerUp.generated.h"

UENUM(BlueprintType)
enum class EPowerUpType : uint8
{
    HealthBoost,
    AttackBoost,
    ExtraMove,
    PowerSteal
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

    void OnPickup(class AChessPieceBase* Piece);
};