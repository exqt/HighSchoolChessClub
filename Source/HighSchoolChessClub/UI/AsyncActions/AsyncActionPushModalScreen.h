// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "UI/Widgets/ModalScreen.h"
#include "AsyncActionPushModalScreen.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FPushModalScreenButtonClickedDelegate,
	FName,
	ButtonType
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPushModalScreenDelegate);

/** Pushes a ModalScreen to UI.Stack.Modal and waits for the user's response. */
UCLASS()
class HIGHSCHOOLCHESSCLUB_API UAsyncActionPushModalScreen : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UI", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
	static UAsyncActionPushModalScreen* PushModalScreenAsync(
		UObject* WorldContextObject,
		FModalScreenInfo ModalScreenInfo,
		TSoftClassPtr<UModalScreen> SoftModalScreenClass
	);

	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable)
	FPushModalScreenButtonClickedDelegate OnButtonClicked;

	/** Called when the modal is closed without selecting a button. */
	UPROPERTY(BlueprintAssignable)
	FPushModalScreenDelegate OnDismissed;

	UPROPERTY(BlueprintAssignable)
	FPushModalScreenDelegate OnFailure;

private:
	void HandleModalCreated(UModalScreen* ModalScreen);
	void HandleButtonClicked(FName ButtonType);
	void HandleModalDeactivated();
	void Complete();
	void Fail();

	UPROPERTY(Transient)
	TObjectPtr<UObject> WorldContextObject;

	UPROPERTY(Transient)
	FModalScreenInfo ModalScreenInfo;

	UPROPERTY(Transient)
	TSoftClassPtr<UModalScreen> SoftModalScreenClass;

	TWeakObjectPtr<UModalScreen> CreatedModalScreen;
	FDelegateHandle ModalDeactivatedHandle;
	bool bCompleted = false;
};
