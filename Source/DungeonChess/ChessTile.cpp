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
    TileMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TileMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    TileMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    GridX = 0;
    GridY = 0;
    OccupyingPiece = nullptr;
}

void AChessTile::Highlight(bool bIsAttackTile)
{
    if (TileMesh)
    {
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
    if (TileMesh && NormalMaterial)
    {
        TileMesh->SetMaterial(0, NormalMaterial);
    }
}