#include "UI/Widgets/Dialogue/DialogueScreen.h"

#include "CommonButtonBase.h"
#include "Components/DynamicEntryBox.h"
#include "Input/CommonUIInputTypes.h"
#include "UI/Core/CCButtonBase.h"
#include "UI/Widgets/Dialogue/DialogueWidget.h"

void UDialogueScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	RegisterAdvanceActionBinding();
}

void UDialogueScreen::NativeConstruct()
{
	Super::NativeConstruct();
	ClearChoices();
}

void UDialogueScreen::NativeOnDeactivated()
{
	DialogueContent->StopTyping();
	ClearChoices();
	Messages.Reset();

	Super::NativeOnDeactivated();
}

UWidget* UDialogueScreen::NativeGetDesiredFocusTarget() const
{
	return DesiredFocusButton ? DesiredFocusButton.Get() : Super::NativeGetDesiredFocusTarget();
}

void UDialogueScreen::PresentDialogue(const TArray<FDialogueMessage>& InMessages)
{
	ClearChoices();
	Messages = InMessages;
	CurrentMessageIndex = 0;
	DisplayCurrentMessage();
}

void UDialogueScreen::AdvanceDialogue()
{
	if (DynamicEntryBox->GetNumEntries() != 0
		&& DynamicEntryBox->GetVisibility() != ESlateVisibility::Collapsed)
	{
		return;
	}

	if (DialogueContent->IsTyping())
	{
		DialogueContent->SkipTyping();
		return;
	}

	if (Messages.IsValidIndex(CurrentMessageIndex + 1))
	{
		++CurrentMessageIndex;
		DisplayCurrentMessage();
		return;
	}

	FinishDialogueSequence();
}

bool UDialogueScreen::ShowChoices(const TArray<FText>& InChoices)
{
	ClearChoices();
	UnregisterAdvanceActionBinding();
	DialogueContent->SetAdvanceActionHidden(true);

	DynamicEntryBox->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	for (int32 ChoiceIndex = 0; ChoiceIndex < InChoices.Num(); ++ChoiceIndex)
	{
		const FText ChoiceText = InChoices[ChoiceIndex];
		UCCButtonBase* ChoiceButton = DynamicEntryBox->CreateEntry<UCCButtonBase>();

		ChoiceButton->SetButtonText(ChoiceText);
		ChoiceButton->OnClicked().AddLambda(
			[this, ChoiceIndex, ChoiceText]()
			{
				DesiredFocusButton = nullptr;
				DynamicEntryBox->SetVisibility(ESlateVisibility::Collapsed);
				RegisterAdvanceActionBinding();
				DialogueContent->SetAdvanceActionHidden(false);
				OnChoiceSelected.Broadcast(ChoiceIndex, ChoiceText);
			});

		if (!DesiredFocusButton)
		{
			DesiredFocusButton = ChoiceButton;
		}
	}

	if (DesiredFocusButton)
	{
		RequestRefreshFocus();
		return true;
	}

	DynamicEntryBox->SetVisibility(ESlateVisibility::Collapsed);
	RegisterAdvanceActionBinding();
	DialogueContent->SetAdvanceActionHidden(false);
	return false;
}

void UDialogueScreen::ClearChoices()
{
	DesiredFocusButton = nullptr;
	RegisterAdvanceActionBinding();
	DialogueContent->SetAdvanceActionHidden(false);
	DynamicEntryBox->Reset<UCCButtonBase>(
		[](UCCButtonBase& ExistingButton)
		{
			ExistingButton.OnClicked().Clear();
		});

	DynamicEntryBox->SetVisibility(ESlateVisibility::Collapsed);
}

void UDialogueScreen::SkipTyping()
{
	DialogueContent->SkipTyping();
}

void UDialogueScreen::RegisterAdvanceActionBinding()
{
	if (AdvanceActionBindingHandle.IsValid())
	{
		return;
	}

	AdvanceActionBindingHandle = RegisterUIActionBinding(FBindUIActionArgs(AdvanceInputActionData, false, FSimpleDelegate::CreateUObject(this, &ThisClass::AdvanceDialogue)));
	DialogueContent->SetAdvanceActionBinding(AdvanceActionBindingHandle);
}

void UDialogueScreen::UnregisterAdvanceActionBinding()
{
	if (!AdvanceActionBindingHandle.IsValid())
	{
		return;
	}

	RemoveActionBinding(AdvanceActionBindingHandle);
	AdvanceActionBindingHandle.Unregister();
}

void UDialogueScreen::DisplayCurrentMessage()
{
	const FDialogueMessage& CurrentMessage = Messages[CurrentMessageIndex];
	const UCCCharacterData* SpeakerData = CurrentMessage.Speaker.LoadSynchronous();
	DialogueContent->PresentMessage(SpeakerData->CharacterName, SpeakerData->Association, CurrentMessage.Message );
}

void UDialogueScreen::FinishDialogueSequence()
{
	Messages.Reset();
	CurrentMessageIndex = INDEX_NONE;
	OnDialogueSequenceFinished.Broadcast();
	DeactivateWidget();
}
