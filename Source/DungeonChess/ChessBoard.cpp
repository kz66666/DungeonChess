// Fill out your copyright notice in the Description page of Project Settings.


#include "ChessBoard.h"
#include "ChessTile.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"

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

                // Determine if this is a light or dark tile (checkerboard pattern)
                bool bIsLightTile = (X + Y) % 2 == 0;

                // Set the appropriate material
                if (NewTile->TileMesh)
                {
                    if (bIsLightTile && NewTile->NormalMaterial)
                    {
                        NewTile->TileMesh->SetMaterial(0, NewTile->NormalMaterial);
                    }
                    else if (!bIsLightTile && NewTile->DarkMaterial)
                    {
                        NewTile->TileMesh->SetMaterial(0, NewTile->DarkMaterial);
                    }
                }

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

    // For 8x8 board with TileSize 100:
    // We want tile centers, not corners
    // Tile [0,0] center should be at (-350, -350)
    // Tile [4,4] center should be at (50, 50)
    // Tile [7,7] center should be at (350, 350)

    float HalfBoardWidth = (BoardWidth * TileSize) / 2.0f;
    float HalfBoardHeight = (BoardHeight * TileSize) / 2.0f;

    // Start at the center of the first tile
    float PosX = (X * TileSize) + (TileSize / 2.0f) - HalfBoardWidth;
    float PosY = (Y * TileSize) + (TileSize / 2.0f) - HalfBoardHeight;

    return BoardOrigin + FVector(PosX, PosY, 0.0f);
}

bool AChessBoard::IsValidPosition(int32 X, int32 Y)
{
    return X >= 0 && X < BoardWidth && Y >= 0 && Y < BoardHeight;
}

FVector AChessBoard::GetWorldLocationForTileFloat(float X, float Y)
{
    FVector BoardOrigin = GetActorLocation();

    // Calculate the center offset
    float HalfWidth = (BoardWidth * TileSize) * 0.5f;
    float HalfHeight = (BoardHeight * TileSize) * 0.5f;

    // Calculate tile position from center using float precision
    float PosX = (X * TileSize) - HalfWidth;
    float PosY = (Y * TileSize) - HalfHeight;

    return BoardOrigin + FVector(PosX, PosY, 0.0f);
}

bool AChessBoard::IsValidPositionFloat(float X, float Y)
{
    return X >= 0.0f && X < static_cast<float>(BoardWidth) &&
        Y >= 0.0f && Y < static_cast<float>(BoardHeight);
}

