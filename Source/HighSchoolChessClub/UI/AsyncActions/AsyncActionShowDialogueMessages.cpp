#include "UI/AsyncActions/AsyncActionShowDialogueMessages.h"

#include "Dialogue/CCDialogueSubsystem.h"

UAsyncActionShowDialogueMessages* UAsyncActionShowDialogueMessages::ShowDialogueMessagesAsync(
	UObject* InWorldContextObject,
	const TArray<FDialogueMessage>& InMessages,
	const TArray<FText>& InChoices)
{
	UAsyncActionShowDialogueMessages* Action = NewObject<UAsyncActionShowDialogueMessages>();
	Action->WorldContextObject = InWorldContextObject;
	Action->Messages = InMessages;
	Action->Choices = InChoices;
	Action->RegisterWithGameInstance(InWorldContextObject);

	return Action;
}

void UAsyncActionShowDialogueMessages::Activate()
{
	Super::Activate();

	if (Messages.IsEmpty())
	{
		Fail();
		return;
	}

	UCCDialogueSubsystem* Subsystem = UCCDialogueSubsystem::Get(WorldContextObject);
	DialogueSubsystem = Subsystem;
	Subsystem->OnDialogueActiveChanged.AddDynamic(this, &ThisClass::HandleDialogueActiveChanged);

	if (UDialogueScreen* Screen = Subsystem->GetDialogueScreen())
	{
		BindDialogueScreen(Screen);
		Subsystem->SetDialogueMessages(Messages);
		ShowChoices();
		return;
	}

	Subsystem->OnDialogueScreenReady.AddDynamic(this, &ThisClass::HandleDialogueScreenReady);
	Subsystem->SetDialogueMessages(Messages);
}

void UAsyncActionShowDialogueMessages::HandleDialogueScreenReady(UDialogueScreen* InDialogueScreen)
{
	DialogueSubsystem->OnDialogueScreenReady.RemoveDynamic(
		this,
		&ThisClass::HandleDialogueScreenReady
	);

	BindDialogueScreen(InDialogueScreen);
	ShowChoices();
}

void UAsyncActionShowDialogueMessages::HandleDialogueActiveChanged(const bool bIsActive)
{
	if (bIsActive || bCompleted)
	{
		return;
	}

	bCompleted = true;
	OnCancelled.Broadcast();
	Complete();
}

void UAsyncActionShowDialogueMessages::HandleDialogueSequenceFinished()
{
	if (bCompleted)
	{
		return;
	}

	bCompleted = true;
	OnCompleted.Broadcast(INDEX_NONE, FText::GetEmpty());
	Complete();
}

void UAsyncActionShowDialogueMessages::HandleChoiceSelected(const int32 ChoiceIndex, const FText ChoiceText)
{
	if (bCompleted)
	{
		return;
	}

	bCompleted = true;
	OnCompleted.Broadcast(ChoiceIndex, ChoiceText);
	Complete();
}

void UAsyncActionShowDialogueMessages::BindDialogueScreen(UDialogueScreen* InDialogueScreen)
{
	DialogueScreen = InDialogueScreen;
	UDialogueScreen* Screen = DialogueScreen;

	if (Choices.IsEmpty())
	{
		Screen->OnDialogueSequenceFinished.AddDynamic(
			this,
			&ThisClass::HandleDialogueSequenceFinished
		);
	}
	else
	{
		Screen->OnChoiceSelected.AddDynamic(this, &ThisClass::HandleChoiceSelected);
	}

	DialogueDeactivatedHandle = Screen->OnDeactivated().AddUObject(
		this,
		&ThisClass::HandleDialogueDeactivated
	);
}

void UAsyncActionShowDialogueMessages::ShowChoices()
{
	if (Choices.IsEmpty())
	{
		return;
	}

	if (!DialogueScreen->ShowChoices(Choices))
	{
		Fail();
	}
}

void UAsyncActionShowDialogueMessages::HandleDialogueDeactivated()
{
	if (bCompleted)
	{
		return;
	}

	bCompleted = true;
	OnCancelled.Broadcast();
	Complete();
}

void UAsyncActionShowDialogueMessages::Complete()
{
	if (UDialogueScreen* Screen = DialogueScreen)
	{
		Screen->OnDialogueSequenceFinished.RemoveDynamic(
			this,
			&ThisClass::HandleDialogueSequenceFinished
		);
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
		Subsystem->OnDialogueScreenReady.RemoveDynamic(
			this,
			&ThisClass::HandleDialogueScreenReady
		);
		Subsystem->OnDialogueActiveChanged.RemoveDynamic(
			this,
			&ThisClass::HandleDialogueActiveChanged
		);
	}

	DialogueSubsystem = nullptr;
	WorldContextObject = nullptr;
	Messages.Reset();
	Choices.Reset();
	SetReadyToDestroy();
}

void UAsyncActionShowDialogueMessages::Fail()
{
	if (bCompleted)
	{
		return;
	}

	bCompleted = true;
	OnFailure.Broadcast();
	Complete();
}
