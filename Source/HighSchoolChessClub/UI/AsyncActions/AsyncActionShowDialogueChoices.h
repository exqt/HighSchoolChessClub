#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncActionShowDialogueChoices.generated.h"

class UDialogueScreen;
class UCCDialogueSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FShowDialogueChoicesSelectedDelegate, int32, ChoiceIndex, FText, ChoiceText );

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FShowDialogueChoicesDelegate);

UCLASS()
class HIGHSCHOOLCHESSCLUB_API UAsyncActionShowDialogueChoices : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Dialogue", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
	static UAsyncActionShowDialogueChoices* ShowDialogueChoicesAsync(
		UObject* WorldContextObject,
		const TArray<FText>& Choices
	);

	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable)
	FShowDialogueChoicesSelectedDelegate OnSelected;

	UPROPERTY(BlueprintAssignable)
	FShowDialogueChoicesDelegate OnCancelled;

	UPROPERTY(BlueprintAssignable)
	FShowDialogueChoicesDelegate OnFailure;

private:
	UFUNCTION()
	void HandleDialogueScreenReady(UDialogueScreen* InDialogueScreen);

	UFUNCTION()
	void HandleDialogueActiveChanged(bool bIsActive);

	UFUNCTION()
	void HandleChoiceSelected(int32 ChoiceIndex, FText ChoiceText);

	void ShowChoices(UDialogueScreen* InDialogueScreen);
	void HandleDialogueDeactivated();
	void Complete();
	void Fail();

	UPROPERTY(Transient)
	TObjectPtr<UObject> WorldContextObject;

	UPROPERTY(Transient)
	TObjectPtr<UCCDialogueSubsystem> DialogueSubsystem;

	UPROPERTY(Transient)
	TObjectPtr<UDialogueScreen> DialogueScreen;

	TArray<FText> Choices;
	FDelegateHandle DialogueDeactivatedHandle;
	bool bCompleted = false;
};
