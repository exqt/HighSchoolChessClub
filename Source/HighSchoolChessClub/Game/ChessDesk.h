#pragma once

#include "CoreMinimal.h"
#include "ChessCoreTypes.h"
#include "GameFramework/Actor.h"
#include "ChessDesk.generated.h"

class AChessPiece;
class USceneComponent;
class UStaticMeshComponent;

/**
 * 체스 게임의 View를 담당하는 Actor
 * - 역할
 *  - ChessPiece Actor들을 관리
 *  - 플레이어 커서 움직임 관리
 */
UCLASS(Blueprintable)
class HIGHSCHOOLCHESSCLUB_API AChessDesk : public AActor
{
	GENERATED_BODY()

public:
	AChessDesk();

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
	bool ApplyMoveToPieceActors(const FChessCoreMove& Move, const FChessCorePiece& MovingPiece, const FChessCorePiece& PieceAfterMove);

protected:
	virtual void BeginPlay() override;

#pragma region Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chess Desk|Cursor")
	TObjectPtr<UStaticMeshComponent> CursorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chess Desk")
	TObjectPtr<USceneComponent> BoardOrigin;
#pragma endregion

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Chess Desk|Cursor")
	FIntPoint CursorSquare = FIntPoint::ZeroValue;

	UPROPERTY(EditDefaultsOnly, Category="Chess Desk")
	TMap<EChessCorePieceType, TSoftClassPtr<AChessPiece>> ChessPieceClasses;

#pragma region Blueprint Events
	UFUNCTION(BlueprintImplementableEvent, Category="Chess Desk|Selection")
	void OnPieceSelected(FIntPoint Square, const TArray<FIntPoint>& LegalDestinations);

	UFUNCTION(BlueprintImplementableEvent, Category="Chess Desk|Selection")
	void OnSelectionCleared();
#pragma endregion

private:
	UPROPERTY(Transient)
	TMap<FIntPoint, TObjectPtr<AChessPiece>> PieceActorsBySquare;

	void RefreshCursorTransform();
	AChessPiece* SpawnPieceActor(const FChessCorePiece& Piece);
	void MovePieceActorToSquare(AChessPiece* PieceActor, FIntPoint Square) const;
	void DestroyPieceActorAtSquare(FIntPoint Square);

	const float SquareSize = 5.0f;
	const float CursorHeight = 1.0f;
};
