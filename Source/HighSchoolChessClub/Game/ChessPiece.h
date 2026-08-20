#pragma once

#include "ChessCoreTypes.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChessPiece.generated.h"

class UMaterialInterface;
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

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chess Piece")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chess Piece")
	TObjectPtr<UStaticMeshComponent> PieceMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chess Piece|Material")
	TObjectPtr<UMaterialInterface> WhiteMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chess Piece|Material")
	TObjectPtr<UMaterialInterface> BlackMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chess Piece|Material")
	TArray<int32> MaterialIndices;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chess Piece")
	EChessCorePieceColor PieceColor = EChessCorePieceColor::White;

private:
	void ApplyPieceColor();
};
