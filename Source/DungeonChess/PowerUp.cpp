// PowerUp.cpp
#include "PowerUp.h"
#include "ChessPieceBase.h"
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
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan,
            FString::Printf(TEXT("PowerUp spawned: %s"),
                PowerUpType == EPowerUpType::ExtraMove ? TEXT("Extra Move") : TEXT("Super Mode")));
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
        // Give the player an additional action this turn
        Piece->bHasActedThisTurn = false;
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Cyan,
                TEXT("EXTRA MOVE! You can act again this turn!"));
        }
        break;

    case EPowerUpType::SuperMode:
        Piece->ActivateSuperMode(SuperModeMovesCount);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta,
                TEXT("*** SUPER MODE ACTIVATED! ***\nEat enemies by moving into them!"));
        }
        break;
    }

    // Destroy the power-up after pickup
    Destroy();
}