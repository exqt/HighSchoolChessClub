// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Widgets/Dialogue/DialogueWidget.h"

#include "CommonTextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "Components/RichTextBlock.h"

void UDialogueWidget::PresentMessage(const FText& InCharacterName, const FText& InAssociation, const FText& InMessage) 
{
	SpeakerName->SetText(InCharacterName);
	SpeakerAssociation->SetText(InAssociation);

	StartTyping(InMessage);
}

void UDialogueWidget::StartTyping(const FText& InText)
{
	ClearTypingTimer();

	FullDialogueString = InText.ToString();
	VisibleCharacterCount = 0;
	DialogueText->SetText(FText::GetEmpty());

	TypeNextCharacter();

	if (VisibleCharacterCount < FullDialogueString.Len())
	{
		GetWorld()->GetTimerManager().SetTimer(
			TypingTimerHandle,
			this,
			&UDialogueWidget::TypeNextCharacter,
			FMath::Max(CharacterInterval, 0.001f),
			true);
	}
}

void UDialogueWidget::SkipTyping()
{
	ClearTypingTimer();
	VisibleCharacterCount = FullDialogueString.Len();
	DialogueText->SetText(FText::FromString(FullDialogueString));

	FinishTyping();
}

void UDialogueWidget::StopTyping()
{
	ClearTypingTimer();
}

bool UDialogueWidget::IsTyping() const
{
	return GetWorld()->GetTimerManager().IsTimerActive(TypingTimerHandle);
}

void UDialogueWidget::NativeDestruct()
{
	ClearTypingTimer();
	Super::NativeDestruct();
}

void UDialogueWidget::TypeNextCharacter()
{
	if (VisibleCharacterCount >= FullDialogueString.Len())
	{
		FinishTyping();
		return;
	}

	const TCHAR CurrentCharacter = FullDialogueString[VisibleCharacterCount];
	++VisibleCharacterCount;
	DialogueText->SetText(FText::FromString(FullDialogueString.Left(VisibleCharacterCount)));

	if (bPlaySoundForWhitespace || !FChar::IsWhitespace(CurrentCharacter))
	{
		UGameplayStatics::PlaySound2D(this, TypingSound, TypingSoundVolume);
	}

	if (VisibleCharacterCount >= FullDialogueString.Len())
	{
		FinishTyping();
	}
}

void UDialogueWidget::FinishTyping()
{
	ClearTypingTimer();
	OnTypingFinished.Broadcast();
}

void UDialogueWidget::ClearTypingTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(TypingTimerHandle);
}
