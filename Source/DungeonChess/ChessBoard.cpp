// Fill out your copyright notice in the Description page of Project Settings.


#include "ChessBoard.h"
#include "ChessTile.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AChessBoard::AChessBoard()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;

    // Create a root component
    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

}

// Called when the game starts or when spawned
void AChessBoard::BeginPlay()
{
    Super::BeginPlay();
    GenerateBoard();
	
}

void AChessBoard::GenerateBoard()
{
    if (!TileClass)
    {
        UE_LOG(LogTemp, Error, TEXT("TileClass not set in ChessBoard!"));
        return;
    }

    Tiles.Empty();

    for (int32 X = 0; X < BoardWidth; X++)
    {
        for (int32 Y = 0; Y < BoardHeight; Y++)
        {
            FVector Location = GetWorldLocationForTile(X, Y);
            FRotator Rotation = FRotator::ZeroRotator;
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;

            AChessTile* NewTile = GetWorld()->SpawnActor<AChessTile>(TileClass, Location, Rotation, SpawnParams);

            if (NewTile)
            {
                NewTile->GridX = X;
                NewTile->GridY = Y;
                Tiles.Add(NewTile);
            }
        }
    }
}

AChessTile* AChessBoard::GetTileAt(int32 X, int32 Y)
{
    if (!IsValidPosition(X, Y))
    {
        return nullptr;
    }

    int32 Index = X * BoardHeight + Y;
    if (Tiles.IsValidIndex(Index))
    {
        return Tiles[Index];
    }

    return nullptr;
}

FVector AChessBoard::GetWorldLocationForTile(int32 X, int32 Y)
{
    FVector BoardOrigin = GetActorLocation();
    float OffsetX = (X - BoardWidth / 2.0f) * TileSize;
    float OffsetY = (Y - BoardHeight / 2.0f) * TileSize;

    return BoardOrigin + FVector(OffsetX, OffsetY, 0.0f);
}

bool AChessBoard::IsValidPosition(int32 X, int32 Y)
{
    return X >= 0 && X < BoardWidth && Y >= 0 && Y < BoardHeight;
}


