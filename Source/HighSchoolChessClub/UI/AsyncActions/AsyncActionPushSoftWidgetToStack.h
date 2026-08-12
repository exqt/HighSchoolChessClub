// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncActionPushSoftWidgetToStack.generated.h"

class UCommonActivatableWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FCCPushSoftWidgetToStackAsyncDelegate,
	UCommonActivatableWidget*,
	Widget
);

/** Widget Soft class를 받아 Async로 지정 Stack에 넣음 */
UCLASS()
class HIGHSCHOOLCHESSCLUB_API UAsyncActionPushSoftWidgetToStack : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UI", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
	static UAsyncActionPushSoftWidgetToStack* PushSoftWidgetToStackAsync(
		UObject* WorldContextObject,
		FGameplayTag WidgetStackTag,
		TSoftClassPtr<UCommonActivatableWidget> SoftWidgetClass
	);

	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable)
	FCCPushSoftWidgetToStackAsyncDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FCCPushSoftWidgetToStackAsyncDelegate OnFailure;

private:
	void Complete(UCommonActivatableWidget* Widget);

	UPROPERTY(Transient)
	TObjectPtr<UObject> WorldContextObject;

	UPROPERTY(Transient)
	TSoftClassPtr<UCommonActivatableWidget> SoftWidgetClass;

	FGameplayTag WidgetStackTag;
};
