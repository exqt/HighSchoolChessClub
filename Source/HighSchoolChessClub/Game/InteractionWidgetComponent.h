#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "InteractionWidgetComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HIGHSCHOOLCHESSCLUB_API UInteractionWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UInteractionWidgetComponent();
	void SetInteractionHovered(bool bIsHovered);

protected:
	virtual void BeginPlay() override;

private:
	void UpdateInteractionText();
};
