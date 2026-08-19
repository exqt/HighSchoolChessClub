// Fill out your copyright notice in the Description page of Project Settings.


#include "FirstPersonPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "FirstPersonPlayerCameraManager.h"
#include "Blueprint/UserWidget.h"

#include "Widgets/Input/SVirtualJoystick.h"

AFirstPersonPlayerController::AFirstPersonPlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = AFirstPersonPlayerCameraManager::StaticClass();
}

void AFirstPersonPlayerController::SetControlMode(EControlMode ControlMode) const
{
	if (!IsLocalPlayerController()) return;
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!Subsystem) return;
	
	Subsystem->RemoveMappingContext(DefaultMappingContext);
	Subsystem->RemoveMappingContext(ChessMappingContext);
	
	if (ControlMode == EControlMode::FirstPerson)
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
	else if (ControlMode == EControlMode::Chess)
	{
		Subsystem->AddMappingContext(ChessMappingContext, 0);
	}
}

void AFirstPersonPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController() && ShouldUseTouchControls())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogTemp, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AFirstPersonPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	SetControlMode(EControlMode::FirstPerson);	
}

bool AFirstPersonPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
