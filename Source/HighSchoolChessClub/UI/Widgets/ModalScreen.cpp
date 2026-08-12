// Fill out your copyright notice in the Description page of Project Settings.


#include "ModalScreen.h"

#include "UI/Core/CCButtonBase.h"
#include "CommonTextBlock.h"
#include "Components/DynamicEntryBox.h"

void UModalScreen::InitializeModalScreen(const FModalScreenInfo& InScreenInfo, TFunction<void(FName)> ClickedButtonCallback)
{
	Title->SetText(InScreenInfo.Title);
	Description->SetText(InScreenInfo.Description);
	
	// 기존 버튼이 남아 있다면 정리
	// 초기화하는 이유: ActivatableContainer는 기존 Widget을 재사용하는 Pooling 기능이 항상 적용됨
	// https://dev.epicgames.com/documentation/unreal-engine/API/Plugins/CommonUI/UCommonActivatableWidgetContaine-/BP_AddWidget
	if (DynamicEntryBox->GetNumEntries() != 0)
	{	
		DynamicEntryBox->Reset<UCCButtonBase>(
			[](UCCButtonBase& ExistingButton) { ExistingButton.OnClicked().Clear(); }
		);
	}

	for (const FModalScreenButtonInfo& AvailableButtonInfo : InScreenInfo.Buttons)
	{	
		UCCButtonBase* AddedButton = DynamicEntryBox->CreateEntry<UCCButtonBase>();
		AddedButton->SetButtonText(AvailableButtonInfo.ButtonTextToDisplay);
		AddedButton->OnClicked().AddLambda(
			[ClickedButtonCallback, AvailableButtonInfo, this]()
			{
				ClickedButtonCallback(AvailableButtonInfo.ButtonType);
				this->DeactivateWidget();
			}
		);
	}
}
