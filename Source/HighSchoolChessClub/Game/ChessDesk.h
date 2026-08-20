#pragma once

#include "CoreMinimal.h"
#include "ChessCoreTypes.h"
#include "GameFramework/Actor.h"
#include "ChessDesk.generated.h"

class AChessPiece;
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
	
	UPROPERTY(EditDefaultsOnly, Category = "Chess Desk")
	TMap<EChessCorePieceType, TSoftClassPtr<AChessPiece>> ChessPieceClasses;
	
	UFUNCTION(BlueprintCallable, Category = "Chess Desk")
	void SetupInitialPosition();
	

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<AChessPiece>> PieceActors;
	
	void RefreshCursorTransform();
};
