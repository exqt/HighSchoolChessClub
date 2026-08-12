// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Core/CCButtonBase.h"
#include "CommonTextBlock.h"
#include "Components/Image.h"

void UCCButtonBase::SetButtonText(FText InText)
{
	if (TextBlockWidget && !InText.IsEmpty())
	{
		TextBlockWidget->SetText(InText);
	}
}

void UCCButtonBase::SetButtonDisplayImage(const FSlateBrush& InBrush)
{
	if (ImageWidget)
	{
		ImageWidget->SetBrush(InBrush);
	}
}

void UCCButtonBase::NativePreConstruct()
{
	Super::NativePreConstruct();
	SetButtonText(ButtonDisplayText);
}
