#pragma once

#include "CoreMinimal.h"
#include "ChessCoreTypes.h"
#include "Game/ChessParticipant.h"
#include "GameFramework/Actor.h"
#include "ChessMatch.generated.h"

class AChessDesk;
class UChessGameState;

UCLASS(Blueprintable)
class HIGHSCHOOLCHESSCLUB_API AChessMatch : public AActor
{
	GENERATED_BODY()

public:
	AChessMatch();

	UFUNCTION(BlueprintCallable, Category="Chess Match|Cursor")
	bool MoveCursor(FIntPoint Delta, EChessPlayerPosition PlayerPosition);

	UFUNCTION(BlueprintCallable, Category="Chess Match|Game")
	bool SelectCurrentSquare(EChessPlayerPosition PlayerPosition);

	UFUNCTION(BlueprintCallable, Category="Chess Match|Game")
	void CancelSelection();

	void BeginPlayerControl(EChessPlayerPosition PlayerPosition);
	void EndPlayerControl(EChessPlayerPosition PlayerPosition);

	UFUNCTION(BlueprintPure, Category="Chess Match|Players")
	bool CanPlayerControl(EChessPlayerPosition PlayerPosition) const;

	UFUNCTION(BlueprintPure, Category="Chess Match|Game")
	bool HasSelectedSquare() const { return bHasSelectedSquare; }

	UFUNCTION(BlueprintPure, Category="Chess Match|Game")
	FIntPoint GetSelectedSquare() const { return SelectedSquare; }

	UFUNCTION(BlueprintPure, Category="Chess Match|Game")
	TArray<FIntPoint> GetLegalDestinationSquares() const;

	UFUNCTION(BlueprintPure, Category="Chess Match|Game")
	EChessCorePieceColor GetSideToMove() const;

	UFUNCTION(BlueprintPure, Category="Chess Match|Players")
	EChessCorePieceColor GetPlayerColor(EChessPlayerPosition PlayerPosition) const;

	UFUNCTION(BlueprintPure, Category="Chess Match|Game")
	UChessGameState* GetChessState() const { return ChessState; }

	UFUNCTION(BlueprintCallable, Category="Chess Match|Game")
	void SetupInitialPosition();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Chess Match")
	TObjectPtr<AChessDesk> Desk;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Chess Match|Players")
	EChessCorePieceColor PlayerAColor = EChessCorePieceColor::White;

	UFUNCTION(BlueprintImplementableEvent, Category="Chess Match|Game")
	void OnMoveApplied(const FChessCoreMove& Move);

private:
	UPROPERTY(Transient)
	TObjectPtr<UChessGameState> ChessState;

	UPROPERTY(Transient)
	TArray<FChessCoreMove> SelectedLegalMoves;

	FIntPoint SelectedSquare = FIntPoint::ZeroValue;
	bool bHasSelectedSquare = false;
	EChessPlayerPosition ActivePlayerPosition = EChessPlayerPosition::PlayerA;
	bool bHasActivePlayer = false;

	bool SelectPieceAtCursor(EChessCorePieceColor PlayerColor);
	bool GetPieceAtSquare(FIntPoint Square, FChessCorePiece& OutPiece) const;
	void RebuildDeskFromState();
	void RefreshCursorVisibility();
};
