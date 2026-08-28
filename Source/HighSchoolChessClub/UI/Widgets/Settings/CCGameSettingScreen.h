// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Input/UIActionBindingHandle.h"
#include "Settings/CCGameSettingRegistry.h"
#include "Widgets/GameSettingScreen.h"
#include "CCGameSettingScreen.generated.h"

class UModalScreen;

/**
 * 
 */
UCLASS()
class HIGHSCHOOLCHESSCLUB_API UCCGameSettingScreen : public UGameSettingScreen
{
	GENERATED_BODY()

public:
	UCCGameSettingScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual bool NativeOnHandleBackAction() override;
	virtual UGameSettingRegistry* CreateRegistry() override;
	virtual void OnSettingsDirtyStateChanged_Implementation(bool bSettingsDirty) override;

	UPROPERTY(EditDefaultsOnly, Category = "CCGameSettingScreen")
	TSoftClassPtr<UModalScreen> ModalScreenClass;

	UPROPERTY(EditDefaultsOnly, Category = "CCGameSettingScreen", meta = (RowType = "/Script/CommonUI.CommonInputActionDataBase"))
	FDataTableRowHandle ApplyInputActionData;

private:
	void HandleApplyAction();
	void ShowChangesModal();

	UFUNCTION()
	void HandleChangesModalButtonClicked(FName ButtonType);

	UFUNCTION()
	void HandleChangesModalClosed();

	FUIActionBindingHandle ApplyActionBindingHandle;
	bool bIsShowingChangesModal = false;
};
