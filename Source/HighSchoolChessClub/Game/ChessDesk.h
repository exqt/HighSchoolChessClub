#pragma once

#include "CoreMinimal.h"
#include "Game/ChessGameTypes.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "SmartObjectTypes.h"
#include "StructUtils/InstancedStruct.h"
#include "ChessDesk.generated.h"

class AChessPiece;
class AChessPlayer;
class UChessClockComponent;
class UChessMatchComponent;
class UInstancedStaticMeshComponent;
class USceneComponent;
class UStaticMeshComponent;

/**
 * 체스 게임의 View를 담당하는 Actor
 * 역할: ChessPiece Actor들을 관리, 플레이어 커서 움직임 관리
 */
UCLASS(Blueprintable)
class HIGHSCHOOLCHESSCLUB_API AChessDesk : public AActor
{
	GENERATED_BODY()

public:
	AChessDesk();

	UFUNCTION(BlueprintPure, Category="ChessDesk")
	UChessMatchComponent* GetChessMatch() const { return ChessMatch; }

	UFUNCTION(BlueprintPure, Category="ChessDesk")
	UChessClockComponent* GetChessClock() const { return ChessClock; }

#pragma region Seats
	UFUNCTION(BlueprintCallable, Category="Chess Desk")
	void RegisterSlot(FSmartObjectSlotHandle SlotHandle);

	UFUNCTION(BlueprintCallable, Category="Chess Desk")
	void SendEventToSlot(FGameplayTag EventTag, const FInstancedStruct& Payload);

	UFUNCTION(BlueprintCallable, Category="Chess Desk")
	void ResetSlots();

	UFUNCTION(BlueprintCallable, Category="Chess Desk")
	bool PullNPCChairIn(AActor* NPC);

	UFUNCTION(BlueprintCallable, Category="Chess Desk")
	bool PullNPCChairOut(AActor* NPC);
#pragma endregion

#pragma region Cursor
	UFUNCTION(BlueprintCallable, Category="Chess Desk|Cursor")
	bool MoveCursor(FIntPoint Delta);

	UFUNCTION(BlueprintCallable, Category="Chess Desk|Cursor")
	bool SetCursorSquare(FIntPoint NewSquare);

	UFUNCTION(BlueprintPure, Category="Chess Desk|Cursor")
	FIntPoint GetCursorSquare() const { return CursorSquare; }

	UFUNCTION(BlueprintPure, Category="Chess Desk|Cursor")
	FVector GetCursorWorldLocation() const;

	UFUNCTION(BlueprintCallable, Category="Chess Desk|Cursor")
	void SetCursorVisible(bool bVisible);

	bool ProjectRayToSquare(const FVector& RayOrigin, const FVector& RayDirection, FIntPoint& OutSquare) const;
	bool WorldLocationToSquare(const FVector& WorldLocation, FIntPoint& OutSquare) const;
#pragma endregion

	void ShowPieceSelection(FIntPoint Square, const TArray<FIntPoint>& LegalDestinations);
	void ClearPieceSelection(FIntPoint Square);
	void RebuildPieceActors(const TArray<FChessCorePiece>& Pieces);
	bool MakeMoveAnimationData(const FChessCoreMove& Move, const FChessCorePiece& MovingPiece, FChessMoveAnimationData& OutData) const;
	bool ApplyMoveToPieceActors(const FChessCoreMove& Move, const FChessCorePiece& MovingPiece, const FChessCorePiece& PieceAfterMove, EChessMoveVisualMode VisualMode = EChessMoveVisualMode::Tween);

protected:
	virtual void BeginPlay() override;

#pragma region Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ChessDesk")
	TObjectPtr<UChessMatchComponent> ChessMatch;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ChessDesk")
	TObjectPtr<UChessClockComponent> ChessClock;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chess Desk|Cursor")
	TObjectPtr<UStaticMeshComponent> CursorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chess Desk")
	TObjectPtr<USceneComponent> BoardOrigin;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chess Desk|Highlight")
	TObjectPtr<UInstancedStaticMeshComponent> LegalMoveCells;
#pragma endregion

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Chess Desk")
	TObjectPtr<AChessPlayer> NPCChessPlayer;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Chess Desk|Cursor")
	FIntPoint CursorSquare = FIntPoint::ZeroValue;

	UPROPERTY(EditDefaultsOnly, Category="Chess Desk")
	TMap<EChessCorePieceType, TSoftClassPtr<AChessPiece>> ChessPieceClasses;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Chess Desk|Highlight")
	FVector LegalMoveCellScale = FVector::OneVector;

#pragma region Blueprint Events
	UFUNCTION(BlueprintImplementableEvent, Category="Chess Desk|Selection")
	void OnPieceSelected(FIntPoint Square, const TArray<FIntPoint>& LegalDestinations);

	UFUNCTION(BlueprintImplementableEvent, Category="Chess Desk|Selection")
	void OnSelectionCleared();
#pragma endregion

private:
	UPROPERTY(Transient)
	TMap<FIntPoint, TObjectPtr<AChessPiece>> PieceActorsBySquare;

	UPROPERTY(Transient)
	TArray<FSmartObjectSlotHandle> RegisteredSlots;

	void RefreshCursorTransform();
	void ShowLegalMoveCells(const TArray<FIntPoint>& Squares);
	void ClearLegalMoveCells();
	FVector SquareToWorldLocation(FIntPoint Square) const;
	AChessPiece* SpawnPieceActor(const FChessCorePiece& Piece);
	void MovePieceActorToSquare(AChessPiece* PieceActor, FIntPoint Square, EChessMoveVisualMode VisualMode) const;
	void DestroyPieceActorAtSquare(FIntPoint Square);

	const float SquareSize = 5.0f;
};
