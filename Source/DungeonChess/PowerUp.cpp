// PowerUp.cpp
#include "PowerUp.h"
#include "ChessPieceBase.h"
#include "PlayerChessPiece.h"
#include "TurnBasedGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"

APowerUp::APowerUp()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create mesh component
    PowerUpMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PowerUpMesh"));
    RootComponent = PowerUpMesh;

    // Set up collision for overlap - use WorldDynamic so it can be found by queries
    PowerUpMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PowerUpMesh->SetCollisionObjectType(ECC_WorldDynamic);
    PowerUpMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    PowerUpMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    PowerUpMesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
    PowerUpMesh->SetGenerateOverlapEvents(true);

    // Bind overlap event
    PowerUpMesh->OnComponentBeginOverlap.AddDynamic(this, &APowerUp::OnOverlapBegin);

    // Default values
    PowerUpType = EPowerUpType::ExtraMove;
    SuperModeMovesCount = 5;
}

void APowerUp::BeginPlay()
{
    Super::BeginPlay();

    // Set mesh and material based on power-up type
    switch (PowerUpType)
    {
    case EPowerUpType::ExtraMove:
        if (ExtraMoveMesh)
        {
            PowerUpMesh->SetStaticMesh(ExtraMoveMesh);
        }
        if (ExtraMoveMaterial)
        {
            PowerUpMesh->SetMaterial(0, ExtraMoveMaterial);
        }
        break;

    case EPowerUpType::SuperMode:
        if (SuperModeMesh)
        {
            PowerUpMesh->SetStaticMesh(SuperModeMesh);
        }
        if (SuperModeMaterial)
        {
            PowerUpMesh->SetMaterial(0, SuperModeMaterial);
        }
        break;

    case EPowerUpType::Revive:
        if (ReviveMesh)
        {
            PowerUpMesh->SetStaticMesh(ReviveMesh);
        }
        if (ReviveMaterial)
        {
            PowerUpMesh->SetMaterial(0, ReviveMaterial);
        }
        break;
    }

    if (GEngine)
    {
        FString PowerUpName;
        switch (PowerUpType)
        {
        case EPowerUpType::ExtraMove:
            PowerUpName = TEXT("Extra Move");
            break;
        case EPowerUpType::SuperMode:
            PowerUpName = TEXT("Super Mode");
            break;
        case EPowerUpType::Revive:
            PowerUpName = TEXT("Revive");
            break;
        default:
            PowerUpName = TEXT("Unknown");
            break;
        }

        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan,
            FString::Printf(TEXT("PowerUp spawned: %s"), *PowerUpName));
    }
}

void APowerUp::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    AChessPieceBase* Piece = Cast<AChessPieceBase>(OtherActor);
    if (Piece)
    {
        OnPickup(Piece);
    }
}

void APowerUp::OnPickup(AChessPieceBase* Piece)
{
    if (!Piece)
    {
        return;
    }

    switch (PowerUpType)
    {
    case EPowerUpType::ExtraMove:
    {
        ATurnBasedGameMode* GameMode = Cast<ATurnBasedGameMode>(GetWorld()->GetAuthGameMode());
        if (GameMode)
        {
            GameMode->bSkipEnemyTurn = true;

            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan,
                    TEXT("ENEMY TURN SKIPPED! You go again!"));
            }
        }
        break;
    }

    case EPowerUpType::SuperMode:
    {
        Piece->ActivateSuperMode(SuperModeMovesCount);

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Magenta,
                TEXT("SUPER MODE ACTIVATED!"));
        }
        break;
    }

    case EPowerUpType::Revive:
    {
        if (APlayerChessPiece* Player = Cast<APlayerChessPiece>(Piece))
        {
            Player->RevivesRemaining++;

            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green,
                    FString::Printf(TEXT("REVIVE OBTAINED! Revives: %d"),
                        Player->RevivesRemaining));
            }
        }
        break;
    }

    default:
        break;
    }

    // Destroy the power-up after pickup
    Destroy();
}