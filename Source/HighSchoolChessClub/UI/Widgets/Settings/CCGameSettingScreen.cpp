// Fill out your copyright notice in the Description page of Project Settings.


#include "CCGameSettingScreen.h"

#include "Input/CommonUIInputTypes.h"
#include "UI/AsyncActions/AsyncActionPushModalScreen.h"
#include "UI/Widgets/ModalScreen.h"
#include "UObject/ConstructorHelpers.h"

#define LOCTEXT_NAMESPACE "CCGameSettingScreen"

namespace CCGameSettingScreen
{
	const FName ApplyButtonType(TEXT("Apply"));
	const FName DiscardButtonType(TEXT("Discard"));
	const FName CancelButtonType(TEXT("Cancel"));
}

UCCGameSettingScreen::UCCGameSettingScreen(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UDataTable> InputActionDataTable(TEXT("/Game/UI/CommonInputData/CCInputActionDataTable.CCInputActionDataTable"));
	ApplyInputActionData.DataTable = InputActionDataTable.Object;
	ApplyInputActionData.RowName = TEXT("ApplySetting");
}

void UCCGameSettingScreen::NativeOnInitialized()
{
	bIsBackHandler = true;
	Super::NativeOnInitialized();

	ApplyActionBindingHandle = RegisterUIActionBinding(FBindUIActionArgs(ApplyInputActionData, false, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleApplyAction)));
	NavigateToSetting(TEXT("AudioCollection"));
}

bool UCCGameSettingScreen::NativeOnHandleBackAction()
{
	if (AttemptToPopNavigation())
	{
		return true;
	}

	if (!HaveSettingsBeenChanged())
	{
		DeactivateWidget();
		return true;
	}

	ShowChangesModal();
	return true;
}

UGameSettingRegistry* UCCGameSettingScreen::CreateRegistry()
{
	return UCCGameSettingRegistry::Get(GetOwningLocalPlayer());
}

void UCCGameSettingScreen::OnSettingsDirtyStateChanged_Implementation(const bool bSettingsDirty)
{
	Super::OnSettingsDirtyStateChanged_Implementation(bSettingsDirty);
	ApplyActionBindingHandle.SetDisplayInActionBar(bSettingsDirty);
}

void UCCGameSettingScreen::HandleApplyAction()
{
	ApplyChanges();
}

void UCCGameSettingScreen::ShowChangesModal()
{
	if (bIsShowingChangesModal || !ensureMsgf(!ModalScreenClass.IsNull(), TEXT("ModalScreenClass is not configured on %s"), *GetName()))
	{
		return;
	}

	FModalScreenButtonInfo ApplyButton;
	ApplyButton.ButtonType = CCGameSettingScreen::ApplyButtonType;
	ApplyButton.ButtonTextToDisplay = LOCTEXT("ApplyButton", "Apply");

	FModalScreenButtonInfo DiscardButton;
	DiscardButton.ButtonType = CCGameSettingScreen::DiscardButtonType;
	DiscardButton.ButtonTextToDisplay = LOCTEXT("DiscardButton", "Discard");

	FModalScreenButtonInfo CancelButton;
	CancelButton.ButtonType = CCGameSettingScreen::CancelButtonType;
	CancelButton.ButtonTextToDisplay = LOCTEXT("CancelButton", "Cancel");

	FModalScreenInfo ModalScreenInfo;
	ModalScreenInfo.Title = LOCTEXT("ChangesModalTitle", "Apply changes?");
	ModalScreenInfo.Description = LOCTEXT("ChangesModalDescription", "There are unsaved setting changes.");
	ModalScreenInfo.Buttons = { ApplyButton, DiscardButton, CancelButton };
	ModalScreenInfo.FocusButtonIndex = 2;

	bIsShowingChangesModal = true;
	UAsyncActionPushModalScreen* ModalAction = UAsyncActionPushModalScreen::PushModalScreenAsync(this, MoveTemp(ModalScreenInfo), ModalScreenClass);
	ModalAction->OnButtonClicked.AddDynamic(this, &ThisClass::HandleChangesModalButtonClicked);
	ModalAction->OnDismissed.AddDynamic(this, &ThisClass::HandleChangesModalClosed);
	ModalAction->OnFailure.AddDynamic(this, &ThisClass::HandleChangesModalClosed);
	ModalAction->Activate();
}

void UCCGameSettingScreen::HandleChangesModalButtonClicked(const FName ButtonType)
{
	bIsShowingChangesModal = false;

	if (ButtonType == CCGameSettingScreen::ApplyButtonType)
	{
		ApplyChanges();
		DeactivateWidget();
	}
	else if (ButtonType == CCGameSettingScreen::DiscardButtonType)
	{
		CancelChanges();
		DeactivateWidget();
	}
}

void UCCGameSettingScreen::HandleChangesModalClosed()
{
	bIsShowingChangesModal = false;
}

#undef LOCTEXT_NAMESPACE
