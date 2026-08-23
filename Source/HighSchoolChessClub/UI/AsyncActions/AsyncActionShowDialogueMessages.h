#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "UI/Widgets/Dialogue/DialogueScreen.h"
#include "AsyncActionShowDialogueMessages.generated.h"

class UCCDialogueSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FShowDialogueMessagesCompletedDelegate,
	int32,
	ChoiceIndex,
	FText,
	ChoiceText
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FShowDialogueMessagesDelegate);

UCLASS()
class HIGHSCHOOLCHESSCLUB_API UAsyncActionShowDialogueMessages : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(
		BlueprintCallable,
		Category = "AsyncActionShowDialogueMessages",
		meta = (
			BlueprintInternalUseOnly = "true",
			WorldContext = "WorldContextObject",
			AutoCreateRefTerm = "Choices"
		)
	)
	static UAsyncActionShowDialogueMessages* ShowDialogueMessagesAsync(
		UObject* WorldContextObject,
		const TArray<FDialogueMessage>& Messages,
		const TArray<FText>& Choices
	);

	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable, Category = "AsyncActionShowDialogueMessages")
	FShowDialogueMessagesCompletedDelegate OnCompleted;

	UPROPERTY(BlueprintAssignable, Category = "AsyncActionShowDialogueMessages")
	FShowDialogueMessagesDelegate OnCancelled;

	UPROPERTY(BlueprintAssignable, Category = "AsyncActionShowDialogueMessages")
	FShowDialogueMessagesDelegate OnFailure;

private:
	UFUNCTION()
	void HandleDialogueScreenReady(UDialogueScreen* InDialogueScreen);

	UFUNCTION()
	void HandleDialogueActiveChanged(bool bIsActive);

	UFUNCTION()
	void HandleDialogueSequenceFinished();

	UFUNCTION()
	void HandleChoiceSelected(int32 ChoiceIndex, FText ChoiceText);

	void BindDialogueScreen(UDialogueScreen* InDialogueScreen);
	void ShowChoices();
	void HandleDialogueDeactivated();
	void Complete();
	void Fail();

	UPROPERTY(Transient)
	TObjectPtr<UObject> WorldContextObject;

	UPROPERTY(Transient)
	TObjectPtr<UCCDialogueSubsystem> DialogueSubsystem;

	UPROPERTY(Transient)
	TObjectPtr<UDialogueScreen> DialogueScreen;

	TArray<FDialogueMessage> Messages;
	TArray<FText> Choices;
	FDelegateHandle DialogueDeactivatedHandle;
	bool bCompleted = false;
};
