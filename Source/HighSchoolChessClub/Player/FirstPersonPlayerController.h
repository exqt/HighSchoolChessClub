// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FirstPersonPlayerController.generated.h"

class UInputMappingContext;

UENUM(BlueprintType)
enum class EControlMode : uint8
{
	FirstPerson = 0,
	Chess = 1,
};

UCLASS()
class HIGHSCHOOLCHESSCLUB_API AFirstPersonPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	AFirstPersonPlayerController();
	
	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetControlMode(EControlMode ControlMode) const;

protected:

	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	UInputMappingContext* DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	UInputMappingContext* ChessMappingContext;

	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;
};
