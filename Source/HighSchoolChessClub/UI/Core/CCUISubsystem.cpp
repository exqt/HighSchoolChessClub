// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Core/CCUISubsystem.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "CommonActivatableWidget.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "UI/Core/CCUISettings.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "CCPrimaryLayout.h"
#include "UI/Widgets/ModalScreen.h"

UCCUISubsystem* UCCUISubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject,EGetWorldErrorMode::Assert);
	return UGameInstance::GetSubsystem<UCCUISubsystem>(World->GetGameInstance());
}

void UCCUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	FWorldDelegates::OnWorldCleanup.AddUObject(this, &ThisClass::HandleWorldCleanup);
}

void UCCUISubsystem::Deinitialize()
{
	FWorldDelegates::OnWorldCleanup.RemoveAll(this);
	ReleasePrimaryLayout();
	Super::Deinitialize();
}

UCCPrimaryLayout* UCCUISubsystem::InitializePrimaryLayout(APlayerController* PlayerController)
{
	if (!IsValid(PlayerController) || !PlayerController->IsLocalPlayerController())
	{
		return nullptr;
	}

	if (CreatedPrimaryLayout && PrimaryLayoutController == PlayerController && PrimaryLayoutWorld == PlayerController->GetWorld())
	{
		if (!CreatedPrimaryLayout->IsInViewport())
		{
			CreatedPrimaryLayout->AddToPlayerScreen();
		}
		return CreatedPrimaryLayout;
	}

	ReleasePrimaryLayout();
	const TSubclassOf<UCCPrimaryLayout> LayoutClass = GetDefault<UCCUIDeveloperSettings>()->PrimaryLayoutClass.LoadSynchronous();
	CreatedPrimaryLayout = CreateWidget<UCCPrimaryLayout>(PlayerController, LayoutClass);
	if (!CreatedPrimaryLayout)
	{
		return nullptr;
	}

	PrimaryLayoutWorld = PlayerController->GetWorld();
	PrimaryLayoutController = PlayerController;
	CreatedPrimaryLayout->AddToPlayerScreen();
	return CreatedPrimaryLayout;
}

void UCCUISubsystem::RegisterCreatedPrimaryLayoutWidget(UCCPrimaryLayout* InCreatedWidget)
{
	if (CreatedPrimaryLayout == InCreatedWidget)
	{
		return;
	}

	ReleasePrimaryLayout();
	CreatedPrimaryLayout = InCreatedWidget;
	PrimaryLayoutWorld = InCreatedWidget ? InCreatedWidget->GetWorld() : nullptr;
	PrimaryLayoutController = InCreatedWidget ? InCreatedWidget->GetOwningPlayer() : nullptr;
}

void UCCUISubsystem::ReleasePrimaryLayout()
{
	++PrimaryLayoutGeneration;
	UCCPrimaryLayout* PreviousLayout = CreatedPrimaryLayout;
	CreatedPrimaryLayout = nullptr;
	PrimaryLayoutWorld.Reset();
	PrimaryLayoutController.Reset();
	if (PreviousLayout)
	{
		PreviousLayout->RemoveFromParent();
	}
}

void UCCUISubsystem::HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	if (PrimaryLayoutWorld == World)
	{
		ReleasePrimaryLayout();
	}
}

void UCCUISubsystem::PushSoftWidgetToStackAsync(const FGameplayTag& InWidgetStackTag, TSoftClassPtr<UCommonActivatableWidget> InSoftWidgetClass, TFunction<void(UCommonActivatableWidget*)> InCallback)
{
	const uint64 RequestGeneration = PrimaryLayoutGeneration;
	const TSharedPtr<FStreamableHandle> LoadHandle = UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(InSoftWidgetClass.ToSoftObjectPath(), FStreamableDelegate::CreateWeakLambda(this, [this, RequestGeneration, InSoftWidgetClass, InWidgetStackTag, InCallback]()
	{
		// A load requested by the previous map must not push content into the new layout.
		if (RequestGeneration != PrimaryLayoutGeneration || !IsValid(CreatedPrimaryLayout) || !PrimaryLayoutWorld.IsValid() || PrimaryLayoutWorld->bIsTearingDown)
		{
			InCallback(nullptr);
			return;
		}

		UClass* LoadedWidgetClass = InSoftWidgetClass.Get();
		UCommonActivatableWidgetContainerBase* FoundWidgetStack = CreatedPrimaryLayout->FindWidgetStackByTag(InWidgetStackTag);
		if (!ensureMsgf(LoadedWidgetClass && IsValid(FoundWidgetStack), TEXT("Failed to load widget or find stack: %s"), *InWidgetStackTag.ToString()))
		{
			InCallback(nullptr);
			return;
		}

		FoundWidgetStack->AddWidget<UCommonActivatableWidget>(LoadedWidgetClass, [InCallback](UCommonActivatableWidget& CreatedWidgetInstance)
		{
			InCallback(&CreatedWidgetInstance);
		});
	}));
	if (!LoadHandle)
	{
		InCallback(nullptr);
	}
}

void UCCUISubsystem::PushModalScreenToModalStack(const FModalScreenInfo& InScreenInfo, TSoftClassPtr<UCommonActivatableWidget> InSoftWidgetClass, TFunction<void(FName)> ButtonClickedCallback, TFunction<void(UModalScreen*)> ModalCreatedCallback)
{
	const FGameplayTag ModalStackTag = FGameplayTag::RequestGameplayTag(FName(TEXT("UI.Stack.Modal")), false);

	PushSoftWidgetToStackAsync(ModalStackTag, InSoftWidgetClass, [ButtonClickedCallback, ModalCreatedCallback, InScreenInfo](UCommonActivatableWidget* PushedWidget)
		{
			if (!PushedWidget)
			{
				if (ModalCreatedCallback)
				{
					ModalCreatedCallback(nullptr);
				}
				return;
			}

			UModalScreen* ModalScreen = CastChecked<UModalScreen>(PushedWidget);

			if (ModalCreatedCallback)
			{
				ModalCreatedCallback(ModalScreen);
			}
			ModalScreen->InitializeModalScreen(InScreenInfo, ButtonClickedCallback);
		}
	);
}
