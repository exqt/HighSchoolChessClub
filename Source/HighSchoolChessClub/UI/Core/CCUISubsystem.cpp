// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Core/CCUISubsystem.h"
#include "Engine/AssetManager.h"
#include "CommonActivatableWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "CCPrimaryLayout.h"
#include "UI/Widgets/ModalScreen.h"

UCCUISubsystem* UCCUISubsystem::Get(const UObject* WorldContextObject)
{
	if (GEngine)
	{
		UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject,EGetWorldErrorMode::Assert);
		return UGameInstance::GetSubsystem<UCCUISubsystem>(World->GetGameInstance());
	}

	return nullptr;
}

void UCCUISubsystem::RegisterCreatedPrimaryLayoutWidget(UCCPrimaryLayout* InCreatedWidget)
{
	check(InCreatedWidget);
	CreatedPrimaryLayout = InCreatedWidget;
}

void UCCUISubsystem::PushSoftWidgetToStackAsync(
	const FGameplayTag& InWidgetStackTag,
	TSoftClassPtr<UCommonActivatableWidget> InSoftWidgetClass, 
	TFunction<void(UCommonActivatableWidget*)> InCallback
) {
	check(!InSoftWidgetClass.IsNull());

	UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
		InSoftWidgetClass.ToSoftObjectPath(),
		FStreamableDelegate::CreateLambda(
			[InSoftWidgetClass, this, InWidgetStackTag, InCallback]()
			{
				UClass* LoadedWidgetClass = InSoftWidgetClass.Get();
				
				check(LoadedWidgetClass && CreatedPrimaryLayout);

				UCommonActivatableWidgetContainerBase* FoundWidgetStack = CreatedPrimaryLayout->FindWidgetStackByTag(InWidgetStackTag);

				UCommonActivatableWidget* CreatedWidget = FoundWidgetStack->AddWidget<UCommonActivatableWidget>(
					LoadedWidgetClass,
					[InCallback](UCommonActivatableWidget& CreatedWidgetInstance) { InCallback(&CreatedWidgetInstance); }
				);
			}
		)
	);
}

void UCCUISubsystem::PushModalScreenToModalStack(
	const FText& InScreenTitle, 
	const FText& InDescription,
	const TArray<FModalScreenButtonInfo>& InButtons,
	TSoftClassPtr<UCommonActivatableWidget> InSoftWidgetClass,
	TFunction<void(FName)> ButtonClickedCallback,
	TFunction<void(UModalScreen*)> ModalCreatedCallback
) {	
	const FGameplayTag ModalStackTag = FGameplayTag::RequestGameplayTag( FName(TEXT("UI.Stack.Modal")), false );

	if (!ModalStackTag.IsValid())
	{
		return;
	}
	
	FModalScreenInfo Info;
	Info.Title = InScreenTitle;
	Info.Description = InDescription;
	Info.Buttons = InButtons;
	
	PushSoftWidgetToStackAsync(
		ModalStackTag,
		InSoftWidgetClass,
		[ButtonClickedCallback, ModalCreatedCallback, Info](UCommonActivatableWidget* PushedWidget)
		{
			UModalScreen* ModalScreen = CastChecked<UModalScreen>(PushedWidget);
			if (ModalCreatedCallback)
			{
				ModalCreatedCallback(ModalScreen);
			}
			ModalScreen->InitializeModalScreen(Info, ButtonClickedCallback);
		}
	);
}
