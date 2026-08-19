// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Core/CCUISubsystem.h"
#include "Engine/AssetManager.h"
#include "CommonActivatableWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "CCPrimaryLayout.h"
#include "UI/Widgets/ModalScreen.h"

UCCUISubsystem* UCCUISubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject,EGetWorldErrorMode::Assert);
	return UGameInstance::GetSubsystem<UCCUISubsystem>(World->GetGameInstance());
}

void UCCUISubsystem::RegisterCreatedPrimaryLayoutWidget(UCCPrimaryLayout* InCreatedWidget)
{
	CreatedPrimaryLayout = InCreatedWidget;
}

void UCCUISubsystem::PushSoftWidgetToStackAsync(
	const FGameplayTag& InWidgetStackTag,
	TSoftClassPtr<UCommonActivatableWidget> InSoftWidgetClass, 
	TFunction<void(UCommonActivatableWidget*)> InCallback
) {
	UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
		InSoftWidgetClass.ToSoftObjectPath(),
		FStreamableDelegate::CreateLambda(
			[InSoftWidgetClass, this, InWidgetStackTag, InCallback]()
			{
				UClass* LoadedWidgetClass = InSoftWidgetClass.Get();
				UCommonActivatableWidgetContainerBase* FoundWidgetStack =
					CreatedPrimaryLayout->FindWidgetStackByTag(InWidgetStackTag);

				FoundWidgetStack->AddWidget<UCommonActivatableWidget>(
					LoadedWidgetClass,
					[InCallback](UCommonActivatableWidget& CreatedWidgetInstance)
					{
						InCallback(&CreatedWidgetInstance);
					}
				);
			}
		)
	);
}

void UCCUISubsystem::PushModalScreenToModalStack(
	const FModalScreenInfo& InScreenInfo,
	TSoftClassPtr<UCommonActivatableWidget> InSoftWidgetClass,
	TFunction<void(FName)> ButtonClickedCallback,
	TFunction<void(UModalScreen*)> ModalCreatedCallback
) {
	const FGameplayTag ModalStackTag = FGameplayTag::RequestGameplayTag(FName(TEXT("UI.Stack.Modal")), false);

	PushSoftWidgetToStackAsync(
		ModalStackTag,
		InSoftWidgetClass,
		[ButtonClickedCallback, ModalCreatedCallback, InScreenInfo](UCommonActivatableWidget* PushedWidget)
		{
			UModalScreen* ModalScreen = CastChecked<UModalScreen>(PushedWidget);

			if (ModalCreatedCallback)
			{
				ModalCreatedCallback(ModalScreen);
			}
			ModalScreen->InitializeModalScreen(InScreenInfo, ButtonClickedCallback);
		}
	);
}
