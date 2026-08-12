// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Core/CCPrimaryLayout.h"

UCommonActivatableWidgetContainerBase* UCCPrimaryLayout::FindWidgetStackByTag(FGameplayTag GameplayTag)
{
	return RegisteredWidgetStackMap.FindRef(GameplayTag);
}

void UCCPrimaryLayout::RegisterWidgetStack(FGameplayTag InStackTag, UCommonActivatableWidgetContainerBase* InStack)
{
	if (!IsDesignTime())
	{
		if (!RegisteredWidgetStackMap.Contains(InStackTag))
		{
			RegisteredWidgetStackMap.Add(InStackTag, InStack);
		}
	}
}
