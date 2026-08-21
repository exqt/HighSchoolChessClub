#include "ChessPlayer.h"

#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "Game/ChessHumanParticipant.h"
#include "Game/ChessMatch.h"
#include "Player/FirstPersonPlayer.h"
#include "Player/FirstPersonPlayerController.h"

AChessPlayer::AChessPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Root"));
	SetRootComponent(SceneRoot);

	ChairMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Chair Mesh"));
	ChairMesh->SetupAttachment(SceneRoot);

	ChairCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Chair Camera"));
	ChairCamera->SetupAttachment(SceneRoot);
	ChairCamera->SetAutoActivate(true);
	ChairCamera->bUsePawnControlRotation = true;

	InteractionName = NSLOCTEXT("ChessPlayer", "InteractionName", "앉기");
}

void AChessPlayer::BeginPlay()
{
	Super::BeginPlay();
	InitialChairCameraRelativeRotation = ChairCamera->GetRelativeRotation();
}

void AChessPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TickCamera(DeltaTime);
}

bool AChessPlayer::EnterPlayer(AFirstPersonPlayer* InExplorationPawn)
{
	if (!IsValid(InExplorationPawn) || IsValid(ExplorationPawn))
	{
		return false;
	}

	UChessHumanParticipant* Participant = ChessMatch
		? ChessMatch->GetHumanParticipant(PlayerPosition)
		: nullptr;
	if (!Participant)
	{
		return false;
	}

	APlayerController* PlayerController = Cast<APlayerController>(InExplorationPawn->GetController());
	if (!IsValid(PlayerController))
	{
		return false;
	}

	ChairCamera->SetRelativeRotation(InitialChairCameraRelativeRotation);

	const FRotator ChairViewRotation = ChairCamera->GetComponentRotation();
	InitialViewRotation = ChairViewRotation;
	ExplorationPawn = InExplorationPawn;
	HumanParticipant = Participant;

	InExplorationPawn->GetCharacterMovement()->StopMovementImmediately();
	InExplorationPawn->GetCharacterMovement()->DisableMovement();

	PlayerController->Possess(this);
	PlayerController->SetControlRotation(ChairViewRotation);

	BeginPlayerView(PlayerController);
	PlayerController->SetViewTargetWithBlend(this, CameraBlendTime, VTBlend_EaseInOut, 2.0f, true);
	HumanParticipant->AttachInputSource(this);

	return true;
}

void AChessPlayer::ReturnToExploration()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!IsValid(PlayerController) || !IsValid(ExplorationPawn))
	{
		return;
	}

	AActor* PreviousViewTarget = PlayerController->GetViewTarget();
	AFirstPersonPlayer* PawnToRestore = ExplorationPawn;

	PlayerController->Possess(PawnToRestore);
	if (AFirstPersonPlayerController* FirstPersonController = Cast<AFirstPersonPlayerController>(PlayerController))
	{
		FirstPersonController->SetControlMode(EControlMode::FirstPerson);
	}

	EndPlayerView(PlayerController);
	HumanParticipant->DetachInputSource(this);
	HumanParticipant = nullptr;

	PlayerController->SetViewTarget(PreviousViewTarget);

	PawnToRestore->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	PawnToRestore->NotifyChessPlayerEnded(this);
	ExplorationPawn = nullptr;

	PlayerController->SetViewTargetWithBlend(PawnToRestore, CameraBlendTime, VTBlend_EaseInOut, 2.0f, true);
}

void AChessPlayer::BeginPlayerView(APlayerController* PlayerController)
{
	ViewingController = PlayerController;

	if (APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager)
	{
		PreviousViewYawMin = CameraManager->ViewYawMin;
		PreviousViewYawMax = CameraManager->ViewYawMax;
		PreviousViewPitchMin = CameraManager->ViewPitchMin;
		PreviousViewPitchMax = CameraManager->ViewPitchMax;

		const FRotator CenterRotation = ChairCamera->GetComponentRotation();
		CameraManager->ViewYawMin = CenterRotation.Yaw - MaxViewYaw;
		CameraManager->ViewYawMax = CenterRotation.Yaw + MaxViewYaw;
		CameraManager->ViewPitchMin = CenterRotation.Pitch + MinViewPitch;
		CameraManager->ViewPitchMax = CenterRotation.Pitch + MaxViewPitch;
	}
}

void AChessPlayer::EndPlayerView(APlayerController* PlayerController)
{
	if (ViewingController.Get() != PlayerController)
	{
		return;
	}

	if (APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager)
	{
		CameraManager->ViewYawMin = PreviousViewYawMin;
		CameraManager->ViewYawMax = PreviousViewYawMax;
		CameraManager->ViewPitchMin = PreviousViewPitchMin;
		CameraManager->ViewPitchMax = PreviousViewPitchMax;
	}

	ViewingController.Reset();
}

void AChessPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &AChessPlayer::LookInput);
		EnhancedInput->BindAction(CursorMoveAction, ETriggerEvent::Started, this, &AChessPlayer::CursorMoveInput);

		EnhancedInput->BindAction(LookHoldAction, ETriggerEvent::Started, this, &AChessPlayer::LookHoldStarted);
		EnhancedInput->BindAction(LookHoldAction, ETriggerEvent::Completed, this, &AChessPlayer::LookHoldEnded);
		EnhancedInput->BindAction(LookHoldAction, ETriggerEvent::Canceled, this, &AChessPlayer::LookHoldEnded);

		EnhancedInput->BindAction(JoystickLookAction, ETriggerEvent::Started, this, &AChessPlayer::StickLookStarted);
		EnhancedInput->BindAction(JoystickLookAction, ETriggerEvent::Triggered, this, &AChessPlayer::StickLookInput);
		EnhancedInput->BindAction(JoystickLookAction, ETriggerEvent::Completed, this, &AChessPlayer::StickLookEnded);
		EnhancedInput->BindAction(JoystickLookAction, ETriggerEvent::Canceled, this, &AChessPlayer::StickLookEnded);

		EnhancedInput->BindAction(SelectAction, ETriggerEvent::Started, this, &AChessPlayer::SelectInput);
		EnhancedInput->BindAction(CancelAction, ETriggerEvent::Started, this, &AChessPlayer::CancelInput);
	}
}

void AChessPlayer::LookInput(const FInputActionValue& Value)
{
	if (!bLookHold) return;

	const FVector2D LookAxis = Value.Get<FVector2D>();
	AddControllerYawInput(LookAxis.X);
	AddControllerPitchInput(LookAxis.Y);
}

void AChessPlayer::CursorMoveInput(const FInputActionValue& Value)
{
	if (!HumanParticipant)
	{
		return;
	}

	const FVector2D Axis = Value.Get<FVector2D>();
	FIntPoint Delta = FIntPoint::ZeroValue;

	// Quantize each analog axis independently so diagonal stick input becomes (±1, ±1).
	if (FMath::Abs(Axis.X) >= CursorMoveThreshold)
	{
		Delta.X = Axis.X > 0.0f ? 1 : -1;
	}

	if (FMath::Abs(Axis.Y) >= CursorMoveThreshold)
	{
		Delta.Y = Axis.Y > 0.0f ? 1 : -1;
	}

	if (Delta != FIntPoint::ZeroValue)
	{
		HumanParticipant->MoveCursor(ConvertInputToBoardDelta(Delta));
	}
}

FIntPoint AChessPlayer::ConvertInputToBoardDelta(const FIntPoint InputDelta) const
{
	if (PlayerPosition == EChessPlayerPosition::PlayerB)
	{
		return FIntPoint(-InputDelta.X, -InputDelta.Y);
	}

	return InputDelta;
}

void AChessPlayer::LookHoldStarted(const FInputActionValue& InputActionValue)
{
	bLookHold = true;
}

void AChessPlayer::LookHoldEnded(const FInputActionValue& InputActionValue)
{
	bLookHold = false;
}

void AChessPlayer::TickCamera(float DeltaTime)
{
	if (bLookHold || bStickLookActive) return;

	AController* PlayerController = GetController();
	if (!PlayerController) return;

	const float ViewReturnInterpSpeed = 5.0f;

	const FRotator CurrentRotation = PlayerController->GetControlRotation();
	const FRotator NewRotation = FMath::RInterpTo(
		CurrentRotation,
		InitialViewRotation,
		DeltaTime,
		ViewReturnInterpSpeed);

	PlayerController->SetControlRotation(NewRotation);

	if (NewRotation.Equals(InitialViewRotation, 0.1f))
	{
		PlayerController->SetControlRotation(InitialViewRotation);
	}
}

void AChessPlayer::StickLookStarted(const FInputActionValue& InputActionValue)
{
	bStickLookActive = true;
}

void AChessPlayer::StickLookInput(const FInputActionValue& InputActionValue)
{
    const FVector2D LookAxis = InputActionValue.Get<FVector2D>();

    AddControllerYawInput(LookAxis.X);
    AddControllerPitchInput(LookAxis.Y);
}

void AChessPlayer::StickLookEnded(const FInputActionValue& InputActionValue)
{
	bStickLookActive = false;
}

void AChessPlayer::SelectInput(const FInputActionValue& InputActionValue)
{
	if (HumanParticipant)
	{
		HumanParticipant->SelectCurrentSquare();
	}
}

void AChessPlayer::CancelInput(const FInputActionValue& InputActionValue)
{
	if (HumanParticipant)
	{
		HumanParticipant->CancelSelection();
	}
}

bool AChessPlayer::CanInteract_Implementation(APawn* Interactor)
{
	return IsValid(Cast<AFirstPersonPlayer>(Interactor))
		&& !IsValid(ExplorationPawn)
		&& ChessMatch
		&& ChessMatch->GetHumanParticipant(PlayerPosition);
}

void AChessPlayer::Interact_Implementation(APawn* Interactor)
{
	if (AFirstPersonPlayer* Player = Cast<AFirstPersonPlayer>(Interactor))
	{
		Player->EnterChessPlayer(this);
	}
}

void AChessPlayer::OnInteractionHover_Implementation(APawn* Interactor)
{
}

void AChessPlayer::OnInteractionUnhover_Implementation(APawn* Interactor)
{
}

FText AChessPlayer::GetInteractionName_Implementation()
{
	return InteractionName;
}
