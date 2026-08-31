#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionActionWidget.generated.h"

UCLASS(Abstract, Blueprintable)
class HIGHSCHOOLCHESSCLUB_API UInteractionActionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction Action Widget")
	void UpdateInteractionText(const FText& InteractionText);
};
