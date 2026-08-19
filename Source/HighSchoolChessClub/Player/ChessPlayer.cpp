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
#include "Game/ChessDesk.h"
#include "Player/FirstPersonPlayer.h"
#include "Player/FirstPersonPlayerController.h"

AChessPlayer::AChessPlayer()
{
	PrimaryActorTick.bCanEverTick = false;

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

bool AChessPlayer::EnterPlayer(AFirstPersonPlayer* InExplorationPawn)
{
	if (!IsValid(InExplorationPawn) || IsValid(ExplorationPawn))
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
	ExplorationPawn = InExplorationPawn;

	InExplorationPawn->GetCharacterMovement()->StopMovementImmediately();
	InExplorationPawn->GetCharacterMovement()->DisableMovement();

	PlayerController->Possess(this);
	PlayerController->SetControlRotation(ChairViewRotation);

	BeginPlayerView(PlayerController);
	PlayerController->SetViewTargetWithBlend(this, CameraBlendTime, VTBlend_EaseInOut, 2.0f, true);

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
	}
}

void AChessPlayer::LookInput(const FInputActionValue& Value)
{
	const FVector2D LookAxis = Value.Get<FVector2D>();
	AddControllerYawInput(LookAxis.X);
	AddControllerPitchInput(LookAxis.Y);
}

void AChessPlayer::CursorMoveInput(const FInputActionValue& Value)
{
	if (!IsValid(ChessDesk))
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
		ChessDesk->MoveCursor(Delta);
	}
}

bool AChessPlayer::CanInteract_Implementation(APawn* Interactor)
{
	return IsValid(Cast<AFirstPersonPlayer>(Interactor)) && !IsValid(ExplorationPawn);
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
