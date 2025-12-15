#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ChessPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class DUNGEONCHESS_API AChessPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
    AChessPlayerController();

    virtual void SetupInputComponent() override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* LeftClickAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* ShowMovesAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* ShowAttacksAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* EndTurnAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* OpenMenuAction;

    UPROPERTY()
    class APlayerChessPiece* ControlledPiece;

    UPROPERTY()
    TArray<class AChessTile*> HighlightedTiles;

    void OnMouseClick(const FInputActionValue& Value);
    void HighlightValidMoves(const FInputActionValue& Value);
    void HighlightAttackTiles(const FInputActionValue& Value);
    void OnEndTurn(const FInputActionValue& Value);
    void ClearHighlights();

    AChessTile* GetTileUnderCursor();

    UFUNCTION()
    void OpenMainMenu();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    bool bShowingMoves = false;
    bool bShowingAttacks = false;
};