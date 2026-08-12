// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ModalScreen.generated.h"

class UDynamicEntryBox;
class UCommonTextBlock;

USTRUCT(BlueprintType)
struct HIGHSCHOOLCHESSCLUB_API FModalScreenButtonInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modal")
	FName ButtonType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modal")
	FText ButtonTextToDisplay;
};

USTRUCT(BlueprintType)
struct HIGHSCHOOLCHESSCLUB_API FModalScreenInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modal")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modal", meta = (MultiLine = "true"))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modal")
	TArray<FModalScreenButtonInfo> Buttons;
};

/**
 * Base widget for modal screens implemented in Blueprint.
 */
UCLASS()
class HIGHSCHOOLCHESSCLUB_API UModalScreen : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	void InitializeModalScreen(const FModalScreenInfo& InScreenInfo, TFunction<void(FName)> ClickedButtonCallback);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Title;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Description;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox> DynamicEntryBox;
};
