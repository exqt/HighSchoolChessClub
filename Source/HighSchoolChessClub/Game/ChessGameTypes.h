#pragma once

#include "ChessCoreTypes.h"
#include "CoreMinimal.h"
#include "ChessPiece.h"
#include "ChessGameTypes.generated.h"

class AActor;

UENUM(BlueprintType)
enum class EChessPlayerPosition : uint8
{
	PlayerA,
	PlayerB
};

enum class EChessMoveVisualMode : uint8
{
	Tween,
	Immediate
};

USTRUCT(BlueprintType)
struct HIGHSCHOOLCHESSCLUB_API FChessMoveAnimationData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 MoveId = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly)
	FIntPoint FromSquare = FIntPoint::ZeroValue;

	UPROPERTY(BlueprintReadOnly)
	FIntPoint ToSquare = FIntPoint::ZeroValue;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AChessPiece> PieceActor = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AChessPiece> CapturedPieceActor = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FIntPoint CapturedSquare = FIntPoint::ZeroValue;

	UPROPERTY(BlueprintReadOnly)
	FVector FromWS = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	FVector ToWS = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	EChessCorePieceType Promotion = EChessCorePieceType::None;
};
