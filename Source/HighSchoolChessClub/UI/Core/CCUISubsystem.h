// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UI/Widgets/ModalScreen.h"
#include "CCUISubsystem.generated.h"

class UCCPrimaryLayout;
class UCommonActivatableWidget;
class APlayerController;
struct FGameplayTag;

UCLASS()
class HIGHSCHOOLCHESSCLUB_API UCCUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	static UCCUISubsystem* Get(const UObject* WorldContextObject);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Call with a valid controller from this game instance after it is ready. Repeated calls reuse its layout. */
	UFUNCTION(BlueprintCallable, Category = "CC UI Subsystem")
	UCCPrimaryLayout* InitializePrimaryLayout(APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable, Category = "CC UI Subsystem")
	void RegisterCreatedPrimaryLayoutWidget(UCCPrimaryLayout* InCreatedWidget);
	
	/** Calls InCallback with nullptr if loading fails or the layout is released before completion. */
	void PushSoftWidgetToStackAsync(const FGameplayTag& InWidgetStackTag, TSoftClassPtr<UCommonActivatableWidget> InSoftWidgetClass, TFunction<void(UCommonActivatableWidget*)> InCallback);
	
	void PushModalScreenToModalStack(const FModalScreenInfo& InScreenInfo, TSoftClassPtr<UCommonActivatableWidget> InSoftWidgetClass, TFunction<void(FName)> ButtonClickedCallback, TFunction<void(UModalScreen*)> ModalCreatedCallback = {});
	
private:
	void ReleasePrimaryLayout();
	void HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);

	UPROPERTY(Transient)
	TObjectPtr<UCCPrimaryLayout> CreatedPrimaryLayout;

	TWeakObjectPtr<UWorld> PrimaryLayoutWorld;
	TWeakObjectPtr<APlayerController> PrimaryLayoutController;
	uint64 PrimaryLayoutGeneration = 0;
};
