// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/AsyncActions/AsyncActionPushSoftWidgetToStack.h"

#include "CommonActivatableWidget.h"
#include "Engine/Engine.h"
#include "UI/Core/CCUISubsystem.h"

UAsyncActionPushSoftWidgetToStack* UAsyncActionPushSoftWidgetToStack::PushSoftWidgetToStackAsync(
	UObject* WorldContextObject,
	FGameplayTag WidgetStackTag,
	TSoftClassPtr<UCommonActivatableWidget> SoftWidgetClass)
{
	UAsyncActionPushSoftWidgetToStack* Action = NewObject<UAsyncActionPushSoftWidgetToStack>();
	Action->WorldContextObject = WorldContextObject;
	Action->WidgetStackTag = WidgetStackTag;
	Action->SoftWidgetClass = MoveTemp(SoftWidgetClass);
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UAsyncActionPushSoftWidgetToStack::Activate()
{
	Super::Activate();

	if (!IsValid(WorldContextObject) || SoftWidgetClass.IsNull() || !WidgetStackTag.IsValid() || !GEngine)
	{
		Complete(nullptr);
		return;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	UCCUISubsystem* UISubsystem = World ? UCCUISubsystem::Get(World) : nullptr;
	if (!UISubsystem)
	{
		Complete(nullptr);
		return;
	}

	TWeakObjectPtr WeakThis(this);
	UISubsystem->PushSoftWidgetToStackAsync(
		WidgetStackTag,
		SoftWidgetClass,
		[WeakThis](UCommonActivatableWidget* Widget)
		{
			if (WeakThis.IsValid())
			{
				WeakThis->Complete(Widget);
			}
		}
	);
}

void UAsyncActionPushSoftWidgetToStack::Complete(UCommonActivatableWidget* Widget)
{
	if (IsValid(Widget))
	{
		OnSuccess.Broadcast(Widget);
	}
	else
	{
		OnFailure.Broadcast(nullptr);
	}

	SetReadyToDestroy();
}
