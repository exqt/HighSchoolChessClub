#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChessDesk.generated.h"

class USceneComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class HIGHSCHOOLCHESSCLUB_API AChessDesk : public AActor
{
	GENERATED_BODY()

public:
	AChessDesk();

	UFUNCTION(BlueprintCallable, Category = "Chess Desk|Cursor")
	bool MoveCursor(FIntPoint Delta);

	UFUNCTION(BlueprintCallable, Category = "Chess Desk|Cursor")
	bool SetCursorSquare(FIntPoint NewSquare);

	UFUNCTION(BlueprintPure, Category = "Chess Desk|Cursor")
	FIntPoint GetCursorSquare() const { return CursorSquare; }

	UFUNCTION(BlueprintPure, Category = "Chess Desk|Cursor")
	FVector GetCursorWorldLocation() const;

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chess Desk|Cursor")
	TObjectPtr<UStaticMeshComponent> CursorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chess Desk")
	TObjectPtr<USceneComponent> BoardOrigin;

	float SquareSize = 5.0f;
	float CursorHeight = 1.0f;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Chess Desk|Cursor")
	FIntPoint CursorSquare = FIntPoint::ZeroValue;

private:
	void RefreshCursorTransform();
};
