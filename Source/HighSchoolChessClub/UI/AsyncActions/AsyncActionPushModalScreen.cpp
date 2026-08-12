// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/AsyncActions/AsyncActionPushModalScreen.h"

#include "Engine/Engine.h"
#include "UI/Core/CCUISubsystem.h"

UAsyncActionPushModalScreen* UAsyncActionPushModalScreen::PushModalScreenAsync(
	UObject* WorldContextObject,
	FModalScreenInfo ModalScreenInfo,
	TSoftClassPtr<UModalScreen> SoftModalScreenClass)
{
	UAsyncActionPushModalScreen* Action = NewObject<UAsyncActionPushModalScreen>();
	Action->WorldContextObject = WorldContextObject;
	Action->ModalScreenInfo = MoveTemp(ModalScreenInfo);
	Action->SoftModalScreenClass = MoveTemp(SoftModalScreenClass);
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UAsyncActionPushModalScreen::Activate()
{
	Super::Activate();

	if (!IsValid(WorldContextObject) || SoftModalScreenClass.IsNull() || !GEngine)
	{
		Fail();
		return;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(
		WorldContextObject,
		EGetWorldErrorMode::ReturnNull
	);
	UCCUISubsystem* UISubsystem = World ? UCCUISubsystem::Get(World) : nullptr;

	if (!UISubsystem)
	{
		Fail();
		return;
	}

	TWeakObjectPtr<UAsyncActionPushModalScreen> WeakThis(this);
	const TSoftClassPtr<UCommonActivatableWidget> ActivatableWidgetClass(
		SoftModalScreenClass.ToSoftObjectPath()
	);

	UISubsystem->PushModalScreenToModalStack(
		ModalScreenInfo.Title,
		ModalScreenInfo.Description,
		ModalScreenInfo.Buttons,
		ActivatableWidgetClass,
		[WeakThis](const FName ButtonType)
		{
			if (WeakThis.IsValid())
			{
				WeakThis->HandleButtonClicked(ButtonType);
			}
		},
		[WeakThis](UModalScreen* ModalScreen)
		{
			if (WeakThis.IsValid())
			{
				WeakThis->HandleModalCreated(ModalScreen);
			}
		}
	);
}

void UAsyncActionPushModalScreen::HandleModalCreated(UModalScreen* ModalScreen)
{
	if (!IsValid(ModalScreen))
	{
		Fail();
		return;
	}

	CreatedModalScreen = ModalScreen;
	ModalDeactivatedHandle = ModalScreen->OnDeactivated().AddUObject(
		this,
		&ThisClass::HandleModalDeactivated
	);
}

void UAsyncActionPushModalScreen::HandleButtonClicked(const FName ButtonType)
{
	if (bCompleted)
	{
		return;
	}

	bCompleted = true;
	OnButtonClicked.Broadcast(ButtonType);
	Complete();
}

void UAsyncActionPushModalScreen::HandleModalDeactivated()
{
	if (bCompleted)
	{
		return;
	}

	bCompleted = true;
	OnDismissed.Broadcast();
	Complete();
}

void UAsyncActionPushModalScreen::Complete()
{
	if (CreatedModalScreen.IsValid() && ModalDeactivatedHandle.IsValid())
	{
		CreatedModalScreen->OnDeactivated().Remove(ModalDeactivatedHandle);
	}

	ModalDeactivatedHandle.Reset();
	CreatedModalScreen.Reset();
	SetReadyToDestroy();
}

void UAsyncActionPushModalScreen::Fail()
{
	if (bCompleted)
	{
		return;
	}

	bCompleted = true;
	OnFailure.Broadcast();
	Complete();
}
