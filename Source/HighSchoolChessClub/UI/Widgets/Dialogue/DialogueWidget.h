// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialogueWidget.generated.h"

struct FUIActionBindingHandle;
class UCommonActionWidget;
class USoundBase;
class UCommonTextBlock;
class URichTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueTypingFinished);

UCLASS(Abstract, Blueprintable)
class HIGHSCHOOLCHESSCLUB_API UDialogueWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetAdvanceActionBinding(FUIActionBindingHandle InAdvanceActionBindingHandle);
	void SetAdvanceActionHidden(bool bHidden);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void PresentMessage(const FText& InCharacterName, const FText& InAssociation, const FText& InMessage);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartTyping(const FText& InText);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SkipTyping();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StopTyping();

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool IsTyping() const;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueTypingFinished OnTypingFinished;

protected:
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<URichTextBlock> DialogueText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> SpeakerName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> SpeakerAssociation;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonActionWidget> AdvanceActionWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Typing", meta = (ClampMin = "0.001", Units = "s"))
	float CharacterInterval = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Sound")
	TObjectPtr<USoundBase> TypingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Sound", meta = (ClampMin = "0.0"))
	float TypingSoundVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Sound")
	bool bPlaySoundForWhitespace = false;

private:
	void TypeNextCharacter();
	void FinishTyping();
	void ClearTypingTimer();

	FString FullDialogueString;
	int32 VisibleCharacterCount = 0;
	FTimerHandle TypingTimerHandle;
};
