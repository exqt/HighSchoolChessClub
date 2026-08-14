// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LocalPlayer.h"
#include "Settings/CCSettingsLocal.h"
#include "CCLocalPlayer.generated.h"

/**
 * 
 */
UCLASS()
class HIGHSCHOOLCHESSCLUB_API UCCLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()
	
public:
	/** Gets the local settings for this player, this is read from config files at process startup and is always valid */
	UFUNCTION()
	UCCSettingsLocal* GetLocalSettings() const;

	// /** Gets the shared setting for this player, this is read using the save game system so may not be correct until after user login */
	// UFUNCTION()
	// UCCSettingsLocal* GetSharedSettings() const;
};
