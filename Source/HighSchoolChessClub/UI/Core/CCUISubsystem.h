// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CCUISubsystem.generated.h"

class UCCPrimaryLayout;
class UCommonActivatableWidget;
struct FGameplayTag;

UCLASS()
class HIGHSCHOOLCHESSCLUB_API UCCUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	static UCCUISubsystem* Get(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable)
	void RegisterCreatedPrimaryLayoutWidget(UCCPrimaryLayout* InCreatedWidget);
	
	void PushSoftWidgetToStackAsync(
		const FGameplayTag& InWidgetStackTag, 
		TSoftClassPtr<UCommonActivatableWidget> InSoftWidgetClass,
		TFunction<void(UCommonActivatableWidget*)> InCallback
	);
	
private:
	UPROPERTY(Transient)
	TObjectPtr<UCCPrimaryLayout> CreatedPrimaryLayout;
};
