#pragma once

#include "CoreMinimal.h"
#include "Game/Interactable.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "Dialogue/CCCharacterData.h"
#include "NPCBase.generated.h"

class UWidgetComponent;

UCLASS(Blueprintable)
class HIGHSCHOOLCHESSCLUB_API ANPCBase : public ACharacter, public IInteractable
{
	GENERATED_BODY()

public:
	ANPCBase();

	virtual void Tick(float DeltaSeconds) override;

	virtual bool CanInteract_Implementation(APawn* Interactor) override;
	virtual FText GetInteractionName_Implementation() override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UWidgetComponent> InteractionWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "0.0", Units = "cm"))
	float InteractionWidgetVisibilityDistance = 140.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	TSoftObjectPtr<UCCCharacterData> CharacterData;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Dialogue")
	bool bIsDialogueActive = false;

private:
	UFUNCTION()
	void HandleDialogueActiveChanged(bool bIsActive);

	void CheckInteractionWidgetVisibilityDistance() const;
};
