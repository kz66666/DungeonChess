// ChessPlayerController.cpp
#include "ChessPlayerController.h"
#include "PlayerChessPiece.h"
#include "ChessBoard.h"
#include "ChessTile.h"
#include "TurnBasedGameMode.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AChessPlayerController::AChessPlayerController()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
}

void AChessPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Get reference to controlled piece
    ControlledPiece = Cast<APlayerChessPiece>(GetPawn());

    // Add Input Mapping Context
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (DefaultMappingContext)
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);

            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Enhanced Input Mapping Context Added"));
            }
        }
        else
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("DefaultMappingContext is NULL!"));
            }
        }
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("Chess Player Controller Initialized"));
    }
}

void AChessPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // Set up action bindings
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
    {
        if (LeftClickAction)
        {
            EnhancedInputComponent->BindAction(LeftClickAction, ETriggerEvent::Triggered, this, &AChessPlayerController::OnMouseClick);

            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("LeftClick Action Bound"));
            }
        }

        if (ShowMovesAction)
        {
            EnhancedInputComponent->BindAction(ShowMovesAction, ETriggerEvent::Triggered, this, &AChessPlayerController::HighlightValidMoves);

            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("ShowMoves Action Bound"));
            }
        }

        if (ShowAttacksAction)
        {
            EnhancedInputComponent->BindAction(ShowAttacksAction, ETriggerEvent::Triggered, this, &AChessPlayerController::HighlightAttackTiles);

            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("ShowAttacks Action Bound"));
            }
        }

        if (EndTurnAction)
        {
            EnhancedInputComponent->BindAction(EndTurnAction, ETriggerEvent::Triggered, this, &AChessPlayerController::OnEndTurn);

            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("EndTurn Action Bound"));
            }
        }

        if (OpenMenuAction)
        {
            EnhancedInputComponent->BindAction(
                OpenMenuAction,
                ETriggerEvent::Triggered,
                this,
                &AChessPlayerController::OpenMainMenu
            );
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Enhanced Input Component not found!"));
        }
    }
}

void AChessPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Optional: Highlight tile under cursor in real-time
}

void AChessPlayerController::OnMouseClick(const FInputActionValue& Value)
{
    if (!ControlledPiece)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("No controlled piece!"));
        }
        return;
    }

    AChessTile* ClickedTile = GetTileUnderCursor();

    if (!ClickedTile)
    {
        ClearHighlights();
        return;
    }

    // Check if we clicked a highlighted tile
    if (HighlightedTiles.Contains(ClickedTile))
    {
        ATurnBasedGameMode* GameMode = Cast<ATurnBasedGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

        if (!GameMode || !GameMode->bPlayerTurn)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Not player's turn!"));
            }
            return;
        }

        if (ControlledPiece->bHasActedThisTurn)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Already acted!"));
            }
            return;
        }

        AChessBoard* Board = GameMode->GameBoard;

        // Super mode - eating enemies by moving into them
        if (ControlledPiece->bSuperModeActive && ClickedTile->OccupyingPiece && bShowingMoves)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Magenta,
                    FString::Printf(TEXT("EATING ENEMY! %d moves left"),
                        ControlledPiece->SuperModeMovesRemaining - 1));
            }

            // Jump attack to eat the enemy
            ControlledPiece->JumpAttackPiece(ClickedTile->GridX, ClickedTile->GridY, Board);

            // Decrease super mode counter
            ControlledPiece->SuperModeMovesRemaining--;
            if (ControlledPiece->SuperModeMovesRemaining <= 0)
            {
                ControlledPiece->DeactivateSuperMode();
            }

            ClearHighlights();
            GameMode->OnPlayerAction();
        }
        // Normal attack (diagonal clash)
        else if (ClickedTile->OccupyingPiece && bShowingAttacks)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red,
                    FString::Printf(TEXT("Clashing with enemy at (%d, %d)!"),
                        ClickedTile->GridX, ClickedTile->GridY));
            }

            ControlledPiece->AttackPiece(ClickedTile->OccupyingPiece);
            ClearHighlights();
            GameMode->OnPlayerAction();
        }
        // Normal movement
        else if (!ClickedTile->OccupyingPiece && bShowingMoves)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
                    FString::Printf(TEXT("Moving to (%d, %d)"), ClickedTile->GridX, ClickedTile->GridY));
            }

            ControlledPiece->MoveToPiece(ClickedTile->GridX, ClickedTile->GridY, Board);
            ClearHighlights();
            GameMode->OnPlayerAction();
        }
    }
    else
    {
        ClearHighlights();
    }
}

void AChessPlayerController::HighlightValidMoves(const FInputActionValue& Value)
{
    if (!ControlledPiece)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("No controlled piece!"));
        }
        return;
    }

    ClearHighlights();

    ATurnBasedGameMode* GameMode = Cast<ATurnBasedGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    if (!GameMode || !GameMode->GameBoard)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("GameMode or Board not found!"));
        }
        return;
    }

    if (!GameMode->bPlayerTurn)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Not player's turn!"));
        }
        return;
    }

    if (ControlledPiece->bHasActedThisTurn)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Already acted this turn!"));
        }
        return;
    }

    TArray<FIntPoint> ValidMoves = ControlledPiece->GetValidMoves(GameMode->GameBoard);

    for (const FIntPoint& Move : ValidMoves)
    {
        AChessTile* Tile = GameMode->GameBoard->GetTileAt(Move.X, Move.Y);
        if (Tile)
        {
            Tile->Highlight(false);
            HighlightedTiles.Add(Tile);
        }
    }

    bShowingMoves = true;
    bShowingAttacks = false;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
            FString::Printf(TEXT("Showing %d valid moves"), ValidMoves.Num()));
    }
}

void AChessPlayerController::HighlightAttackTiles(const FInputActionValue& Value)
{
    if (!ControlledPiece)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("No controlled piece!"));
        }
        return;
    }

    ClearHighlights();

    ATurnBasedGameMode* GameMode = Cast<ATurnBasedGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    if (!GameMode || !GameMode->GameBoard)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("GameMode or Board not found!"));
        }
        return;
    }

    if (!GameMode->bPlayerTurn)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Not player's turn!"));
        }
        return;
    }

    if (ControlledPiece->bHasActedThisTurn)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Already acted this turn!"));
        }
        return;
    }

    TArray<FIntPoint> AttackTiles = ControlledPiece->GetAttackTiles(GameMode->GameBoard);

    for (const FIntPoint& Tile : AttackTiles)
    {
        AChessTile* ChessTile = GameMode->GameBoard->GetTileAt(Tile.X, Tile.Y);
        if (ChessTile)
        {
            ChessTile->Highlight(true);
            HighlightedTiles.Add(ChessTile);
        }
    }

    bShowingAttacks = true;
    bShowingMoves = false;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red,
            FString::Printf(TEXT("Showing %d attack tiles"), AttackTiles.Num()));
    }
}

void AChessPlayerController::ClearHighlights()
{
    for (AChessTile* Tile : HighlightedTiles)
    {
        if (Tile)
        {
            Tile->ResetHighlight();
        }
    }

    int32 Count = HighlightedTiles.Num();
    HighlightedTiles.Empty();
    bShowingMoves = false;
    bShowingAttacks = false;

    if (Count > 0 && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, TEXT("Highlights cleared"));
    }
}

AChessTile* AChessPlayerController::GetTileUnderCursor()
{
    FHitResult HitResult;
    GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

    return Cast<AChessTile>(HitResult.GetActor());
}

void AChessPlayerController::OnEndTurn(const FInputActionValue& Value)
{
    ATurnBasedGameMode* GameMode = Cast<ATurnBasedGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    if (GameMode)
    {
        if (!GameMode->bPlayerTurn)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Not player's turn!"));
            }
            return;
        }

        ClearHighlights();

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Yellow, TEXT("===== ENDING TURN ====="));
        }

        // Manually mark as acted to allow turn progression
        if (ControlledPiece)
        {
            ControlledPiece->bHasActedThisTurn = true;
        }

        GameMode->OnPlayerAction();
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("GameMode not found!"));
        }
    }
}

void AChessPlayerController::OpenMainMenu()
{
    UGameplayStatics::OpenLevel(this, FName("MainMenuLevel"));
}
