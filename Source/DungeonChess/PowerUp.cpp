// PowerUp.cpp
#include "PowerUp.h"
#include "ChessPieceBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

APowerUp::APowerUp()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create mesh component
    PowerUpMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PowerUpMesh"));
    RootComponent = PowerUpMesh;

    // Set up collision
    PowerUpMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PowerUpMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    PowerUpMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Default power-up type
    PowerUpType = EPowerUpType::HealthBoost;
}

void APowerUp::OnPickup(AChessPieceBase* Piece)
{
    if (!Piece)
    {
        return;
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Magenta,
            FString::Printf(TEXT("Power-up picked up: %s"), *UEnum::GetValueAsString(PowerUpType)));
    }

    switch (PowerUpType)
    {
    case EPowerUpType::HealthBoost:
        Piece->Health += 50;
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
                FString::Printf(TEXT("Health increased! New health: %d"), Piece->Health));
        }
        break;

    case EPowerUpType::AttackBoost:
        Piece->AttackPower += 10;
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red,
                FString::Printf(TEXT("Attack increased! New attack: %d"), Piece->AttackPower));
        }
        break;

    case EPowerUpType::ExtraMove:
        Piece->MovementRange += 1;
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan,
                FString::Printf(TEXT("Movement range increased! New range: %d"), Piece->MovementRange));
        }
        break;

    case EPowerUpType::PowerSteal:
        // Give the piece a temporary buff or special ability
        Piece->AttackPower += 5;
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Purple,
                TEXT("Power Steal ability activated!"));
        }
        break;
    }

    // Destroy the power-up after pickup
    Destroy();
}