#pragma once

#include "ChessCoreTypes.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChessPiece.generated.h"

class UMaterialInterface;
class UPositionTweenComponent;
class USceneComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class HIGHSCHOOLCHESSCLUB_API AChessPiece : public AActor
{
	GENERATED_BODY()

public:
	AChessPiece();

	UFUNCTION(BlueprintCallable, Category="Chess Piece")
	void SetPieceColor(EChessCorePieceColor NewColor);

	UFUNCTION(BlueprintPure, Category="Chess Piece")
	EChessCorePieceColor GetPieceColor() const { return PieceColor; }

	UFUNCTION(BlueprintCallable, Category="Chess Piece|Tween")
	void LiftPiece();

	UFUNCTION(BlueprintCallable, Category="Chess Piece|Tween")
	void LowerPiece();

	UFUNCTION(BlueprintCallable, Category="Chess Piece|Tween")
	void MovePieceTo(FVector TargetWorldPosition);

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chess Piece")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chess Piece")
	TObjectPtr<UStaticMeshComponent> PieceMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chess Piece|Tween")
	TObjectPtr<UPositionTweenComponent> PositionTween;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chess Piece|Material")
	TObjectPtr<UMaterialInterface> WhiteMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chess Piece|Material")
	TObjectPtr<UMaterialInterface> BlackMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chess Piece|Material")
	TArray<int32> MaterialIndices;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chess Piece")
	EChessCorePieceColor PieceColor = EChessCorePieceColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chess Piece|Tween", meta=(ClampMin="0.0"))
	float SelectionLiftHeight = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chess Piece|Tween", meta=(ClampMin="0.0"))
	float SelectionTweenDuration = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chess Piece|Tween", meta=(ClampMin="0.0"))
	float MoveTweenDuration = 0.25f;

private:
	void ApplyPieceColor();

	FVector RestingWorldPosition = FVector::ZeroVector;
	bool bIsLifted = false;
};
