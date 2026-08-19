#include "Dialogue/CCDialogueSubsystem.h"

#include "Dialogue/CCDialogueSettings.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UI/Core/CCUISubsystem.h"
#include "UI/Widgets/Dialogue/DialogueScreen.h"

UCCDialogueSubsystem* UCCDialogueSubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	return World->GetSubsystem<UCCDialogueSubsystem>();
}

void UCCDialogueSubsystem::StartDialogue()
{
	if (bIsDialogueActive || bIsLoadingDialogueScreen || IsValid(DialogueScreen))
	{
		return;
	}

	const UCCDialogueSettings* Settings = GetDefault<UCCDialogueSettings>();
	const TSoftClassPtr<UDialogueScreen> DialogueScreenClass = Settings->DialogueScreenClass;
	UCCUISubsystem* UISubsystem = UCCUISubsystem::Get(this);

	SetDialogueActive(true);
	bIsLoadingDialogueScreen = true;
	const TSoftClassPtr<UCommonActivatableWidget> ActivatableDialogueScreenClass(DialogueScreenClass);
	const FGameplayTag LayerTag = FGameplayTag::RequestGameplayTag(FName(TEXT("UI.Stack.InGame")));

	UISubsystem->PushSoftWidgetToStackAsync(
		LayerTag,
		ActivatableDialogueScreenClass,
		[this](UCommonActivatableWidget* PushedWidget)
		{
			DialogueScreen = CastChecked<UDialogueScreen>(PushedWidget);
			DialogueScreen->OnActivated().AddUObject(
				this,
				&ThisClass::HandleDialogueScreenActivated
			);
			DialogueScreen->OnDeactivated().AddUObject(
				this,
				&ThisClass::HandleDialogueScreenDeactivated
			);
		}
	);
}

void UCCDialogueSubsystem::HandleDialogueScreenActivated()
{
	UDialogueScreen* ActivatedScreen = DialogueScreen;
	bIsLoadingDialogueScreen = false;

	ActivatedScreen->OnActivated().RemoveAll(this);
	if (!bIsDialogueActive)
	{
		ActivatedScreen->DeactivateWidget();
		return;
	}

	if (!PendingDialogueMessages.IsEmpty())
	{
		ActivatedScreen->PresentDialogue(PendingDialogueMessages);
		PendingDialogueMessages.Reset();
	}

	OnDialogueScreenReady.Broadcast(ActivatedScreen);
}

void UCCDialogueSubsystem::HandleDialogueScreenDeactivated()
{
	DialogueScreen->OnActivated().RemoveAll(this);
	DialogueScreen->OnDeactivated().RemoveAll(this);
	DialogueScreen = nullptr;
	bIsLoadingDialogueScreen = false;
	SetDialogueActive(false);
}

void UCCDialogueSubsystem::EndDialogue()
{
	SetDialogueActive(false);

	if (IsValid(DialogueScreen) && DialogueScreen->IsActivated())
	{
		DialogueScreen->DeactivateWidget();
	}
}

void UCCDialogueSubsystem::SetDialogueMessages(const TArray<FDialogueMessage>& InMessages)
{
	if (bIsDialogueActive && IsValid(DialogueScreen) && DialogueScreen->IsActivated())
	{
		DialogueScreen->PresentDialogue(InMessages);
		PendingDialogueMessages.Reset();
		return;
	}

	PendingDialogueMessages = InMessages;
}

void UCCDialogueSubsystem::SetDialogueActive(bool bActive)
{
	if (bIsDialogueActive == bActive)
	{
		return;
	}

	bIsDialogueActive = bActive;
	OnDialogueActiveChanged.Broadcast(bIsDialogueActive);
}
