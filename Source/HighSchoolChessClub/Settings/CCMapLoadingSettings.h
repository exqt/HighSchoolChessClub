#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CCMapLoadingSettings.generated.h"

class UWorld;

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Map Loading"))
class HIGHSCHOOLCHESSCLUB_API UCCMapLoadingSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "Map Loading")
	TSoftObjectPtr<UWorld> TitleLevel;

	UPROPERTY(Config, EditAnywhere, Category = "Map Loading")
	TSoftObjectPtr<UWorld> GameLevel;
};
