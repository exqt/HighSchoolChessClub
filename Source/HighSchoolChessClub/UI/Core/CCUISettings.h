#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CCUISettings.generated.h"

class UCCPrimaryLayout;
class UDialogueScreen;

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "UI"))
class HIGHSCHOOLCHESSCLUB_API UCCUIDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "CC UI Developer Settings")
	TSoftClassPtr<UCCPrimaryLayout> PrimaryLayoutClass;

	UPROPERTY(Config, EditAnywhere, Category = "CC UI Developer Settings")
	TSoftClassPtr<UDialogueScreen> DialogueScreenClass;
};
