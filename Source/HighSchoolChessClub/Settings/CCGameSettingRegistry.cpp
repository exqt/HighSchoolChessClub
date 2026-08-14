// Fill out your copyright notice in the Description page of Project Settings.


#include "CCGameSettingRegistry.h"

UCCGameSettingRegistry::UCCGameSettingRegistry()
{
}

UCCGameSettingRegistry* UCCGameSettingRegistry::Get(ULocalPlayer* InLocalPlayer)
{
	UCCGameSettingRegistry* Registry = FindObject<UCCGameSettingRegistry>(InLocalPlayer, TEXT("CCGameSettingRegistry"), EFindObjectFlags::ExactClass);
	if (Registry == nullptr)
	{
		Registry = NewObject<UCCGameSettingRegistry>(InLocalPlayer, TEXT("CCGameSettingRegistry"));
		Registry->Initialize(InLocalPlayer);
	}

	return Registry;
}

void UCCGameSettingRegistry::SaveChanges()
{
	Super::SaveChanges();
}

void UCCGameSettingRegistry::OnInitialize(ULocalPlayer* InLocalPlayer)
{
	Super::OnInitialize(InLocalPlayer);
}

bool UCCGameSettingRegistry::IsFinishedInitializing() const
{
	return Super::IsFinishedInitializing();
}

// UGameSettingCollection* UCCGameSettingRegistry::InitializeVideoSettings(ULocalPlayer* InLocalPlayer)
// {
// }
//
// void UCCGameSettingRegistry::AddPerformanceStatPage(UGameSettingCollection* Screen, ULocalPlayer* InLocalPlayer)
// {
// }
//
// UGameSettingCollection* UCCGameSettingRegistry::InitializeGameplaySettings(ULocalPlayer* InLocalPlayer)
// {
// }
//
// UGameSettingCollection* UCCGameSettingRegistry::InitializeMouseAndKeyboardSettings(ULocalPlayer* InLocalPlayer)
// {
// }
//
// UGameSettingCollection* UCCGameSettingRegistry::InitializeGamepadSettings(ULocalPlayer* InLocalPlayer)
// {
// }
