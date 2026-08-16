// Fill out your copyright notice in the Description page of Project Settings.


#include "FirstPersonPlayer.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "Game/InteractionComponent.h"
#include "ChessPlayer.h"

AFirstPersonPlayer::AFirstPersonPlayer()
{
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);
	
	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(GetMesh());
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;
	
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("Interaction Component"));

	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
}

void AFirstPersonPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFirstPersonPlayer::MoveInput);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFirstPersonPlayer::LookInput);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AFirstPersonPlayer::Interact);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void AFirstPersonPlayer::MoveInput(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	DoMove(MovementVector.X, MovementVector.Y);
}

void AFirstPersonPlayer::LookInput(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	DoAim(LookAxisVector.X, LookAxisVector.Y);
}

void AFirstPersonPlayer::Interact()
{
	InteractionComponent->TryInteract();
}

bool AFirstPersonPlayer::SitDown(AActor* SeatActor)
{
	return EnterChessPlayer(Cast<AChessPlayer>(SeatActor), false);
}

bool AFirstPersonPlayer::StartPlayingChess(AActor* ViewTargetActor)
{
	return EnterChessPlayer(Cast<AChessPlayer>(ViewTargetActor), true);
}

void AFirstPersonPlayer::StopPlayingChess()
{
}

void AFirstPersonPlayer::StandUp()
{
	if (IsValid(CurrentChessPlayer))
	{
		CurrentChessPlayer->ReturnToExploration();
	}
}

bool AFirstPersonPlayer::IsSeated() const
{
	return false;
}

void AFirstPersonPlayer::NotifyChessPlayerEnded(AChessPlayer* EndedPawn)
{
	if (CurrentChessPlayer == EndedPawn)
	{
		CurrentChessPlayer = nullptr;
	}
}

void AFirstPersonPlayer::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

bool AFirstPersonPlayer::EnterChessPlayer(AChessPlayer* InChessPlayer, bool bPlayingChess)
{
	const EChessPlayerMode NewMode = bPlayingChess ? EChessPlayerMode::PlayingChess : EChessPlayerMode::Seated;
	if (!InChessPlayer->EnterPlayer(this))
	{
		return false;
	}

	CurrentChessPlayer = InChessPlayer;
	return true;
}

void AFirstPersonPlayer::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}
