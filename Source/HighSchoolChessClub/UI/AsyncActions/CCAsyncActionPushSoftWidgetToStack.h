// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "CCAsyncActionPushSoftWidgetToStack.generated.h"

class UCommonActivatableWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FCCPushSoftWidgetToStackAsyncDelegate,
	UCommonActivatableWidget*,
	Widget
);

/** Pushes a soft widget class to a registered UI stack after loading it asynchronously. */
UCLASS()
class HIGHSCHOOLCHESSCLUB_API UCCAsyncActionPushSoftWidgetToStack : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "CC|UI", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
	static UCCAsyncActionPushSoftWidgetToStack* PushSoftWidgetToStackAsync(
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
