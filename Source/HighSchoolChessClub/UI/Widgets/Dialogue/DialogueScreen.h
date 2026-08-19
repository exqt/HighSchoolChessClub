#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Dialogue/CCCharacterData.h"
#include "UI/Core/CCButtonBase.h"
#include "DialogueScreen.generated.h"

class UCommonButtonBase;
class UDialogueWidget;
class UDynamicEntryBox;

USTRUCT(BlueprintType)
struct HIGHSCHOOLCHESSCLUB_API FDialogueMessage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TSoftObjectPtr<UCCCharacterData> Speaker;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (MultiLine = "true"))
	FText Message;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnDialogueChoiceSelected, int32, ChoiceIndex, FText, ChoiceText );
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueSequenceFinished);

UCLASS(Abstract, Blueprintable)
class HIGHSCHOOLCHESSCLUB_API UDialogueScreen : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void PresentDialogue(const TArray<FDialogueMessage>& InMessages);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void AdvanceDialogue();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool ShowChoices(const TArray<FText>& InChoices);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void ClearChoices();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SkipTyping();

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueChoiceSelected OnChoiceSelected;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueSequenceFinished OnDialogueSequenceFinished;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	virtual FReply NativeOnMouseButtonDown( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	void DisplayCurrentMessage();
	void FinishDialogueSequence();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDialogueWidget> DialogueContent;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox> DynamicEntryBox;

	UPROPERTY(Transient)
	TObjectPtr<UCCButtonBase> DesiredFocusButton;

	TArray<FDialogueMessage> Messages;
	int32 CurrentMessageIndex = INDEX_NONE;
};
