#include "UI/AsyncActions/AsyncActionShowDialogueChoices.h"

#include "Dialogue/CCDialogueSubsystem.h"
#include "UI/Widgets/Dialogue/DialogueScreen.h"

UAsyncActionShowDialogueChoices* UAsyncActionShowDialogueChoices::ShowDialogueChoicesAsync(
	UObject* InWorldContextObject,
	const TArray<FText>& InChoices)
{
	UAsyncActionShowDialogueChoices* Action = NewObject<UAsyncActionShowDialogueChoices>();
	Action->WorldContextObject = InWorldContextObject;
	Action->Choices = InChoices;
	Action->RegisterWithGameInstance(InWorldContextObject);

	return Action;
}

void UAsyncActionShowDialogueChoices::Activate()
{
	Super::Activate();

	UCCDialogueSubsystem* Subsystem = UCCDialogueSubsystem::Get(WorldContextObject);
	DialogueSubsystem = Subsystem;
	Subsystem->OnDialogueActiveChanged.AddDynamic(this, &ThisClass::HandleDialogueActiveChanged);

	if (UDialogueScreen* Screen = Subsystem->GetDialogueScreen())
	{
		ShowChoices(Screen);
		return;
	}

	Subsystem->OnDialogueScreenReady.AddDynamic(this, &ThisClass::HandleDialogueScreenReady);
}

void UAsyncActionShowDialogueChoices::HandleDialogueScreenReady(UDialogueScreen* InDialogueScreen)
{
	ShowChoices(InDialogueScreen);
}

void UAsyncActionShowDialogueChoices::HandleDialogueActiveChanged(const bool bIsActive)
{
	if (bIsActive || bCompleted)
	{
		return;
	}

	bCompleted = true;
	OnCancelled.Broadcast();
	Complete();
}

void UAsyncActionShowDialogueChoices::ShowChoices(UDialogueScreen* InDialogueScreen)
{
	DialogueSubsystem->OnDialogueScreenReady.RemoveDynamic(this, &ThisClass::HandleDialogueScreenReady);

	DialogueScreen = InDialogueScreen;
	UDialogueScreen* Screen = DialogueScreen;
	Screen->OnChoiceSelected.AddDynamic(this, &ThisClass::HandleChoiceSelected);
	DialogueDeactivatedHandle = Screen->OnDeactivated().AddUObject(
		this,
		&ThisClass::HandleDialogueDeactivated
	);

	Screen->ShowChoices(Choices);
}

void UAsyncActionShowDialogueChoices::HandleChoiceSelected(const int32 ChoiceIndex, const FText ChoiceText) 
{
	if (bCompleted)
	{
		return;
	}

	bCompleted = true;
	OnSelected.Broadcast(ChoiceIndex, ChoiceText);
	Complete();
}

void UAsyncActionShowDialogueChoices::HandleDialogueDeactivated()
{
	if (bCompleted)
	{
		return;
	}

	bCompleted = true;
	OnCancelled.Broadcast();
	Complete();
}

void UAsyncActionShowDialogueChoices::Complete()
{
	if (UDialogueScreen* Screen = DialogueScreen)
	{
		Screen->OnChoiceSelected.RemoveDynamic(this, &ThisClass::HandleChoiceSelected);

		if (DialogueDeactivatedHandle.IsValid())
		{
			Screen->OnDeactivated().Remove(DialogueDeactivatedHandle);
		}
	}

	DialogueDeactivatedHandle.Reset();
	DialogueScreen = nullptr;

	if (UCCDialogueSubsystem* Subsystem = DialogueSubsystem)
	{
		Subsystem->OnDialogueScreenReady.RemoveDynamic(this, &ThisClass::HandleDialogueScreenReady);
		Subsystem->OnDialogueActiveChanged.RemoveDynamic(this, &ThisClass::HandleDialogueActiveChanged);
	}

	DialogueSubsystem = nullptr;
	WorldContextObject = nullptr;
	Choices.Reset();
	SetReadyToDestroy();
}

void UAsyncActionShowDialogueChoices::Fail()
{
	if (bCompleted)
	{
		return;
	}

	bCompleted = true;
	OnFailure.Broadcast();
	Complete();
}
