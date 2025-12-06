// PlayerChessPiece.h
#pragma once

#include "CoreMinimal.h"
#include "ChessPieceBase.h"
#include "PlayerChessPiece.generated.h"

UCLASS()
class DUNGEONCHESS_API APlayerChessPiece : public AChessPieceBase
{
    GENERATED_BODY()

public:
    APlayerChessPiece();

    // Add these function declarations
    virtual TArray<FIntPoint> GetValidMoves(class AChessBoard* Board) override;
    virtual TArray<FIntPoint> GetAttackTiles(class AChessBoard* Board) override;

    // Player can attack diagonals
    bool CanAttackDiagonal(int32 TargetX, int32 TargetY);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* FollowCamera;

};