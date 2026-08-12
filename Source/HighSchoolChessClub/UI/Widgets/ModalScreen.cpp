// Fill out your copyright notice in the Description page of Project Settings.


#include "ModalScreen.h"

#include "UI/Core/CCButtonBase.h"
#include "CommonTextBlock.h"
#include "Components/DynamicEntryBox.h"

void UModalScreen::InitializeModalScreen(const FModalScreenInfo& InScreenInfo, TFunction<void(FName)> ClickedButtonCallback)
{
	Title->SetText(InScreenInfo.Title);
	Description->SetText(InScreenInfo.Description);
	DesiredFocusButton = nullptr;
	
	// 기존 버튼이 남아 있다면 정리
	// 초기화하는 이유: ActivatableContainer는 기존 Widget을 재사용하는 Pooling 기능이 항상 적용됨
	// https://dev.epicgames.com/documentation/unreal-engine/API/Plugins/CommonUI/UCommonActivatableWidgetContaine-/BP_AddWidget
	if (DynamicEntryBox->GetNumEntries() != 0)
	{	
		DynamicEntryBox->Reset<UCCButtonBase>(
			[](UCCButtonBase& ExistingButton) { ExistingButton.OnClicked().Clear(); }
		);
	}

	const int32 DesiredFocusIndex = InScreenInfo.Buttons.IsValidIndex(InScreenInfo.FocusButtonIndex)
		? InScreenInfo.FocusButtonIndex
		: 0;

	for (int32 ButtonIndex = 0; ButtonIndex < InScreenInfo.Buttons.Num(); ++ButtonIndex)
	{
		const FModalScreenButtonInfo& AvailableButtonInfo = InScreenInfo.Buttons[ButtonIndex];
		UCCButtonBase* AddedButton = DynamicEntryBox->CreateEntry<UCCButtonBase>();
		AddedButton->SetButtonText(AvailableButtonInfo.ButtonTextToDisplay);

		if (ButtonIndex == DesiredFocusIndex)
		{
			DesiredFocusButton = AddedButton;
		}

		AddedButton->OnClicked().AddLambda(
			[ClickedButtonCallback, AvailableButtonInfo, this]()
			{
				ClickedButtonCallback(AvailableButtonInfo.ButtonType);
				this->DeactivateWidget();
			}
		);
	}

	if (DesiredFocusButton)
	{
		RequestRefreshFocus();
	}
}

UWidget* UModalScreen::NativeGetDesiredFocusTarget() const
{
	return DesiredFocusButton ? DesiredFocusButton.Get() : Super::NativeGetDesiredFocusTarget();
}
