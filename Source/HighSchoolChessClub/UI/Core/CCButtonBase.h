// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "CCButtonBase.generated.h"


class UCommonTextBlock;
class UImage;

/**
 * 
 */
UCLASS(Abstract)
class HIGHSCHOOLCHESSCLUB_API UCCButtonBase : public UCommonButtonBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SetButtonText(FText InText);

	UFUNCTION(BlueprintCallable)
	void SetButtonDisplayImage(const FSlateBrush& InBrush);

private:
	//~ Begin UUserWidget Interface
	virtual void NativePreConstruct() override;
	//~ End UUserWidget Interface
	
	//~ Begin UCommonButtonBase Interface
	//~ End UCommonButtonBase Interface
	
	//**** Bound Widgets ****//
	UPROPERTY(meta = (BindWidgetOptional))
	UCommonTextBlock* TextBlockWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional,AllowPrivateAccess = "true"))
	UImage* ImageWidget;
	//**** Bound Widgets ****//

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Button", meta = (AllowPrivateAccess = "true"))
	FText ButtonDisplayText;
};

