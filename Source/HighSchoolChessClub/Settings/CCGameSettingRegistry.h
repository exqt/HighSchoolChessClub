// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameSettingRegistry.h"

#include "CCGameSettingRegistry.generated.h"

#define GET_LOCAL_SETTINGS_FUNCTION_PATH(FunctionOrPropertyName)							\
	MakeShared<FGameSettingDataSourceDynamic>(TArray<FString>({								\
		GET_FUNCTION_NAME_STRING_CHECKED(UCCLocalPlayer, GetLocalSettings),				\
		GET_FUNCTION_NAME_STRING_CHECKED(UCCSettingsLocal, FunctionOrPropertyName)		\
	}))
	
/**
 * 
 */
UCLASS()
class HIGHSCHOOLCHESSCLUB_API UCCGameSettingRegistry : public UGameSettingRegistry
{
	GENERATED_BODY()
	
public:
	UCCGameSettingRegistry();

	static UCCGameSettingRegistry* Get(ULocalPlayer* InLocalPlayer);
	
	virtual void SaveChanges() override;

protected:
	virtual void OnInitialize(ULocalPlayer* InLocalPlayer) override;
	virtual bool IsFinishedInitializing() const override;

	UGameSettingCollection* InitializeAudioSettings(ULocalPlayer* InLocalPlayer);
	// UGameSettingCollection* InitializeVideoSettings(ULocalPlayer* InLocalPlayer);
	// UGameSettingCollection* InitializeGameplaySettings(ULocalPlayer* InLocalPlayer);
	// UGameSettingCollection* InitializeMouseAndKeyboardSettings(ULocalPlayer* InLocalPlayer);
	// UGameSettingCollection* InitializeGamepadSettings(ULocalPlayer* InLocalPlayer);

	UPROPERTY()
	TObjectPtr<UGameSettingCollection> VideoSettings;

	UPROPERTY()
	TObjectPtr<UGameSettingCollection> AudioSettings;

	UPROPERTY()
	TObjectPtr<UGameSettingCollection> GameplaySettings;

	UPROPERTY()
	TObjectPtr<UGameSettingCollection> MouseAndKeyboardSettings;

	UPROPERTY()
	TObjectPtr<UGameSettingCollection> GamepadSettings;
};
