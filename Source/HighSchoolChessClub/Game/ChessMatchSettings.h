#pragma once

#include "CoreMinimal.h"
#include "GameEnums.h"
#include "ChessMatchSettings.generated.h"

USTRUCT(BlueprintType)
struct HIGHSCHOOLCHESSCLUB_API FChessMatchSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess Match", meta = (ClampMin = "0"))
	int32 InitialTimeSeconds = 600;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess Match", meta = (ClampMin = "0"))
	int32 IncrementSeconds = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess Match", meta = (ClampMin = "1", ClampMax = "20"))
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess Match")
	EChessPlayerColor ColorSelection = EChessPlayerColor::Random;
};
