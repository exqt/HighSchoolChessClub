#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ChessParticipant.generated.h"

class AActor;
class AChessDesk;

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

UINTERFACE(BlueprintType)
class HIGHSCHOOLCHESSCLUB_API UChessParticipant : public UInterface
{
	GENERATED_BODY()
};

class HIGHSCHOOLCHESSCLUB_API IChessParticipant
{
	GENERATED_BODY()

public:
};
