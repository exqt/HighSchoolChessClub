// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Settings/CCGameSettingRegistry.h"
#include "Widgets/GameSettingScreen.h"
#include "CCGameSettingScreen.generated.h"

/**
 * 
 */
UCLASS()
class HIGHSCHOOLCHESSCLUB_API UCCGameSettingScreen : public UGameSettingScreen
{
	GENERATED_BODY()
	
private:
	virtual UGameSettingRegistry* CreateRegistry() override
	{
		return UCCGameSettingRegistry::Get(GetOwningLocalPlayer());
	}

	virtual void NativeOnInitialized() override
	{
		Super::NativeOnInitialized();

		NavigateToSetting(TEXT("AudioCollection"));
	}
};
