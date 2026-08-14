// Fill out your copyright notice in the Description page of Project Settings.


#include "CCLocalPlayer.h"

UCCSettingsLocal* UCCLocalPlayer::GetLocalSettings() const
{
	return UCCSettingsLocal::Get();
}
