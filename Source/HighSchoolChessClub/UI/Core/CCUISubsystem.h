// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UI/Widgets/ModalScreen.h"
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
	
	void PushModalScreenToModalStack(
		const FText& InScreenTitle, 
		const FText& InDescription,
		const TArray<FModalScreenButtonInfo>& InButtons,
		TSoftClassPtr<UCommonActivatableWidget> InSoftWidgetClass,
		TFunction<void(FName)> ButtonClickedCallback,
		TFunction<void(UModalScreen*)> ModalCreatedCallback = {}
	);
	
private:
	UPROPERTY(Transient)
	TObjectPtr<UCCPrimaryLayout> CreatedPrimaryLayout;
};
