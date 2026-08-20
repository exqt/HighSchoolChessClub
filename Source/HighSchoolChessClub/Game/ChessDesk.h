#pragma once

#include "CoreMinimal.h"
#include "ChessCoreTypes.h"
#include "Game/ChessParticipant.h"
#include "GameFramework/Actor.h"
#include "ChessDesk.generated.h"

class AChessPiece;
class UChessGameState;
class USceneComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class HIGHSCHOOLCHESSCLUB_API AChessDesk : public AActor
{
	GENERATED_BODY()

public:
	AChessDesk();

#pragma region Cursor
	UFUNCTION(BlueprintCallable, Category = "Chess Desk|Cursor")
	bool MoveCursor(FIntPoint Delta, EChessPlayerPosition PlayerPosition);

	UFUNCTION(BlueprintCallable, Category = "Chess Desk|Cursor")
	bool SetCursorSquare(FIntPoint NewSquare);

	UFUNCTION(BlueprintPure, Category = "Chess Desk|Cursor")
	FIntPoint GetCursorSquare() const { return CursorSquare; }

	UFUNCTION(BlueprintPure, Category = "Chess Desk|Cursor")
	FVector GetCursorWorldLocation() const;

	UFUNCTION(BlueprintCallable, Category = "Chess Desk|Game")
	bool SelectCurrentSquare(EChessPlayerPosition PlayerPosition);

	UFUNCTION(BlueprintCallable, Category = "Chess Desk|Game")
	void CancelSelection();
#pragma endregion

	void BeginPlayerControl(EChessPlayerPosition PlayerPosition);
	void EndPlayerControl(EChessPlayerPosition PlayerPosition);

	UFUNCTION(BlueprintPure, Category = "Chess Desk|Players")
	bool CanPlayerControl(EChessPlayerPosition PlayerPosition) const;

	UFUNCTION(BlueprintPure, Category = "Chess Desk|Game")
	bool HasSelectedSquare() const { return bHasSelectedSquare; }

	UFUNCTION(BlueprintPure, Category = "Chess Desk|Game")
	FIntPoint GetSelectedSquare() const { return SelectedSquare; }

	UFUNCTION(BlueprintPure, Category = "Chess Desk|Game")
	TArray<FIntPoint> GetLegalDestinationSquares() const;

	UFUNCTION(BlueprintPure, Category = "Chess Desk|Game")
	EChessCorePieceColor GetSideToMove() const;

	UFUNCTION(BlueprintPure, Category = "Chess Desk|Players")
	EChessCorePieceColor GetPlayerColor(EChessPlayerPosition PlayerPosition) const;

protected:
	virtual void BeginPlay() override;

#pragma region Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chess Desk|Cursor")
	TObjectPtr<UStaticMeshComponent> CursorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chess Desk")
	TObjectPtr<USceneComponent> BoardOrigin;
#pragma endregion

	float SquareSize = 5.0f;
	float CursorHeight = 1.0f;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Chess Desk|Cursor")
	FIntPoint CursorSquare = FIntPoint::ZeroValue;

	UPROPERTY(EditDefaultsOnly, Category = "Chess Desk")
	TMap<EChessCorePieceType, TSoftClassPtr<AChessPiece>> ChessPieceClasses;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Chess Desk|Players")
	EChessCorePieceColor PlayerAColor = EChessCorePieceColor::White;

	UFUNCTION(BlueprintCallable, Category = "Chess Desk")
	void SetupInitialPosition();

#pragma region Blueprint Events
	UFUNCTION(BlueprintImplementableEvent, Category = "Chess Desk|Game")
	void OnPieceSelected(FIntPoint Square, const TArray<FIntPoint>& LegalDestinations);

	UFUNCTION(BlueprintImplementableEvent, Category = "Chess Desk|Game")
	void OnSelectionCleared();

	UFUNCTION(BlueprintImplementableEvent, Category = "Chess Desk|Game")
	void OnMoveApplied(const FChessCoreMove& Move);
#pragma endregion

private:
#pragma region Game State
	UPROPERTY(Transient)
	TObjectPtr<UChessGameState> ChessState;

	UPROPERTY(Transient)
	TArray<FChessCoreMove> SelectedLegalMoves;

	FIntPoint SelectedSquare = FIntPoint::ZeroValue;
	bool bHasSelectedSquare = false;

	EChessPlayerPosition ActivePlayerPosition = EChessPlayerPosition::PlayerA;
	bool bHasActivePlayer = false;
#pragma endregion

	UPROPERTY(Transient)
	TMap<FIntPoint, TObjectPtr<AChessPiece>> PieceActorsBySquare;

	void RefreshCursorTransform();
	void RefreshCursorVisibility();

	/**
	 * 존재 하는 모든 Piece 액터를 제거 후 현재 보드 상태로 새로 구성
	 */
	void RebuildPieceActorsFromState();

	AChessPiece* SpawnPieceActor(const FChessCorePiece& Piece);
	bool ApplyMoveToPieceActors(const FChessCoreMove& Move, const FChessCorePiece& MovingPiece);
	void MovePieceActorToSquare(AChessPiece* PieceActor, FIntPoint Square) const;
	void DestroyPieceActorAtSquare(FIntPoint Square);
	bool SelectPieceAtCursor(EChessCorePieceColor PlayerColor);
	bool GetPieceAtSquare(FIntPoint Square, FChessCorePiece& OutPiece) const;
};
