#pragma once

#include "CoreMinimal.h"
#include "ChessParticipantTypes.generated.h"

class AActor;

UENUM(BlueprintType)
enum class EChessPlayerPosition : uint8
{
	PlayerA,
	PlayerB
};

USTRUCT(BlueprintType)
struct HIGHSCHOOLCHESSCLUB_API FChessMoveAnimationData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Chess")
	FIntPoint FromSquare = FIntPoint::ZeroValue;

	UPROPERTY(BlueprintReadOnly, Category="Chess")
	FIntPoint ToSquare = FIntPoint::ZeroValue;

	UPROPERTY(BlueprintReadOnly, Category="Chess")
	TObjectPtr<AActor> PieceActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="Chess")
	FVector FromWS = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="Chess")
	FVector ToWS = FVector::ZeroVector;
};
