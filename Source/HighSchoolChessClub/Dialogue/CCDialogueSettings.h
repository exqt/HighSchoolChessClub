#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CCDialogueSettings.generated.h"

class UDialogueScreen;

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Dialogue"))
class HIGHSCHOOLCHESSCLUB_API UCCDialogueSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue")
	TSoftClassPtr<UDialogueScreen> DialogueScreenClass;
};
