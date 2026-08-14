// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "CCSettingsLocal.generated.h"

class USoundControlBusMix;
class USoundControlBus;

/**
 * 
 */
UCLASS()
class HIGHSCHOOLCHESSCLUB_API UCCSettingsLocal : public UGameUserSettings
{
	GENERATED_BODY()
	
public:
	static UCCSettingsLocal* Get();
	
#pragma region Audio
public:
	UPROPERTY(Transient)
	TObjectPtr<USoundControlBusMix> ControlBusMix = nullptr;

	UPROPERTY(Transient)
	bool bSoundControlBusMixLoaded;
	
	UPROPERTY(Transient)
	TMap<FName/*SoundClassName*/, TObjectPtr<USoundControlBus>> ControlBusMap;
	
	UFUNCTION()
	float GetOverallVolume() const { return OverallVolume; }
	
	UFUNCTION()
	float GetMusicVolume() const { return MusicVolume; }
	
	UFUNCTION()
	float GetSFXVolume() const { return SFXVolume; }
	
	UFUNCTION()
	void SetOverallVolume(float InVolume);
	
	UFUNCTION()
	void SetMusicVolume(float InVolume);
	
	UFUNCTION()
	void SetSFXVolume(float InVolume);
	
private:
	void LoadUserControlBusMix();
	void SetVolumeForControlBus(USoundControlBus* InSoundControlBus, float InVolume);
	
	UPROPERTY(Config)
	float OverallVolume = 1.0f;
	
	UPROPERTY(Config)
	float MusicVolume = 1.0f;

	UPROPERTY(Config)
	float SFXVolume = 1.0f;
#pragma endregion
	
};
