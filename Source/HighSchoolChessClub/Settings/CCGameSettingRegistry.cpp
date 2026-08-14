// Fill out your copyright notice in the Description page of Project Settings.


#include "CCGameSettingRegistry.h"
#include "GameSettingCollection.h"

#include "Player/CCLocalPlayer.h"

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
	
	if (UCCSettingsLocal* Settings = UCCSettingsLocal::Get())
	{
		Settings->SaveSettings();
	}
}

void UCCGameSettingRegistry::OnInitialize(ULocalPlayer* InLocalPlayer)
{
	UCCLocalPlayer* CCLocalPlayer = Cast<UCCLocalPlayer>(InLocalPlayer);

	AudioSettings = InitializeAudioSettings(CCLocalPlayer);
	RegisterSetting(AudioSettings);
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
