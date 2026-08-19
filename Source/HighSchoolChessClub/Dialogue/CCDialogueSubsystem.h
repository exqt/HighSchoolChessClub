#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "UI/Widgets/Dialogue/DialogueScreen.h"
#include "CCDialogueSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCCDialogueActiveChanged, bool, bIsActive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCCDialogueScreenReady, UDialogueScreen*, DialogueScreen);

UCLASS()
class HIGHSCHOOLCHESSCLUB_API UCCDialogueSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UCCDialogueSubsystem* Get(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogue();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void EndDialogue();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SetDialogueMessages(const TArray<FDialogueMessage>& InMessages);

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool IsDialogueActive() const { return bIsDialogueActive; }

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	UDialogueScreen* GetDialogueScreen() const
	{
		return IsValid(DialogueScreen) && DialogueScreen->IsActivated()
			? DialogueScreen
			: nullptr;
	}

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FCCDialogueActiveChanged OnDialogueActiveChanged;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FCCDialogueScreenReady OnDialogueScreenReady;

private:
	void SetDialogueActive(bool bActive);
	void HandleDialogueScreenActivated();
	void HandleDialogueScreenDeactivated();

	UPROPERTY(Transient)
	TObjectPtr<UDialogueScreen> DialogueScreen;

	TArray<FDialogueMessage> PendingDialogueMessages;

	bool bIsDialogueActive = false;
	bool bIsLoadingDialogueScreen = false;
};
