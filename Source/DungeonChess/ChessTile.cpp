// ChessTile.cpp
#include "ChessTile.h"
#include "ChessPieceBase.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AChessTile::AChessTile()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create the tile mesh component
    TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
    RootComponent = TileMesh;

    // Set up collision
    TileMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    TileMesh->SetCollisionObjectType(ECC_WorldStatic);
    TileMesh->SetCollisionResponseToAllChannels(ECR_Block);
    TileMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    GridX = 0;
    GridY = 0;
    OccupyingPiece = nullptr;
    OriginalMaterial = nullptr;
}

void AChessTile::Highlight(bool bIsAttackTile)
{
    if (TileMesh)
    {
        // Store original material if not already stored
        if (!OriginalMaterial)
        {
            OriginalMaterial = TileMesh->GetMaterial(0);
        }

        if (bIsAttackTile && AttackHighlightMaterial)
        {
            TileMesh->SetMaterial(0, AttackHighlightMaterial);
        }
        else if (HighlightedMaterial)
        {
            TileMesh->SetMaterial(0, HighlightedMaterial);
        }
    }
}

void AChessTile::ResetHighlight()
{
    if (TileMesh && OriginalMaterial)
    {
        TileMesh->SetMaterial(0, OriginalMaterial);
    }
}