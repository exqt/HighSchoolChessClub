#include "ChessPlayer.h"

#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "CommonInputSubsystem.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "Game/ChessDesk.h"
#include "Game/ChessHumanParticipant.h"
#include "Game/ChessMatch.h"
#include "Game/PositionTweenComponent.h"
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

	SeatAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("Seat Anchor"));
	SeatAnchor->SetupAttachment(SceneRoot);

	PositionTween = CreateDefaultSubobject<UPositionTweenComponent>(TEXT("Position Tween"));

	InteractionName = NSLOCTEXT("ChessPlayer", "InteractionName", "앉기");
}

void AChessPlayer::BeginPlay()
{
	Super::BeginPlay();
	InitialChairCameraRelativeRotation = ChairCamera->GetRelativeRotation();
	ChairOutWorldLocation = GetActorLocation();
	if (ChessMatch)
	{
		if (IsHumanSeat())
		{
			ChessMatch->RegisterParticipant(PlayerPosition, this);
		}
	}
}

void AChessPlayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ChessMatch)
	{
		UChessParticipant* Participant = ChessMatch->GetParticipant(PlayerPosition);
		if (Participant && Participant->GetPerformer() == this)
		{
			ChessMatch->UnregisterParticipant(Participant);
		}
	}
	Super::EndPlay(EndPlayReason);
}

void AChessPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TickCamera(DeltaTime);
	UpdateChairMove();
}

void AChessPlayer::EnterPlayer(AFirstPersonPlayer* InExplorationPawn)
{
	UChessHumanParticipant* Participant = ChessMatch->GetHumanParticipant(PlayerPosition);
	APlayerController* PlayerController = Cast<APlayerController>(InExplorationPawn->GetController());

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

	// TODO: Possess 시 Blend가 되지 않음
	PlayerController->SetViewTargetWithBlend(this, CameraBlendTime, VTBlend_EaseInOut, 2.0f, true);

	PullChairIn();
	ChessMatch->EnterMatchSetup();
}

void AChessPlayer::RequestStartMatch()
{
	ChessMatch->RequestStartMatch();
}

void AChessPlayer::UnPossessed()
{
	Super::UnPossessed();

	if (ChessMatch && HumanParticipant)
	{
		ChessMatch->NotifyHumanPlayerUnpossessed(HumanParticipant);
	}
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
	HumanParticipant = nullptr;

	PlayerController->SetViewTarget(PreviousViewTarget);

	PawnToRestore->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	PawnToRestore->NotifyChessPlayerEnded(this);
	PullChairOut();
	ExplorationPawn = nullptr;

	PlayerController->SetViewTargetWithBlend(PawnToRestore, CameraBlendTime, VTBlend_EaseInOut, 2.0f, true);
}

void AChessPlayer::BeginPlayerView(APlayerController* PlayerController)
{
	bPreviousShowMouseCursor = PlayerController->bShowMouseCursor;
	CommonInputSubsystem = UCommonInputSubsystem::Get(PlayerController->GetLocalPlayer());
	CommonInputSubsystem->OnInputMethodChangedNative.AddUObject(this, &AChessPlayer::HandleInputMethodChanged);
	HandleInputMethodChanged(CommonInputSubsystem->GetCurrentInputType());

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
	if (APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager)
	{
		CameraManager->ViewYawMin = PreviousViewYawMin;
		CameraManager->ViewYawMax = PreviousViewYawMax;
		CameraManager->ViewPitchMin = PreviousViewPitchMin;
		CameraManager->ViewPitchMax = PreviousViewPitchMax;
	}

	CommonInputSubsystem->OnInputMethodChangedNative.RemoveAll(this);
	CommonInputSubsystem = nullptr;
	PlayerController->bShowMouseCursor = bPreviousShowMouseCursor;
	PlayerController->SetInputMode(FInputModeGameOnly());
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

#pragma region Input Bindings
void AChessPlayer::LookInput(const FInputActionValue& Value)
{
	if (!bLookHold) return;

	const FVector2D LookAxis = Value.Get<FVector2D>();
	AddControllerYawInput(LookAxis.X);
	AddControllerPitchInput(LookAxis.Y);
}

void AChessPlayer::CursorMoveInput(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	FIntPoint Delta = FIntPoint::ZeroValue;

	float CursorMoveThreshold = 0.5f;

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

void AChessPlayer::LookHoldStarted(const FInputActionValue& InputActionValue)
{
	bLookHold = true;
}

void AChessPlayer::LookHoldEnded(const FInputActionValue& InputActionValue)
{
	bLookHold = false;
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
	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	// 컨트롤러가 아닌 마우스를 사용하여 클릭하였다면
	if (PlayerController->IsInputKeyDown(EKeys::LeftMouseButton))
	{
		UpdateCursorFromMouse();
	}

	HumanParticipant->SelectCurrentSquare();
}

void AChessPlayer::CancelInput(const FInputActionValue& InputActionValue)
{
	HumanParticipant->CancelSelection();
}
#pragma endregion

FIntPoint AChessPlayer::ConvertInputToBoardDelta(const FIntPoint InputDelta) const
{
	if (PlayerPosition == EChessPlayerPosition::PlayerB)
	{
		return FIntPoint(-InputDelta.X, -InputDelta.Y);
	}

	return InputDelta;
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

void AChessPlayer::UpdateCursorFromMouse() const
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	FVector RayOrigin;
	FVector RayDirection;

	if (!PlayerController->DeprojectMousePositionToWorld(RayOrigin, RayDirection))
	{
		return;
	}

	FIntPoint Square;
	const AChessDesk* Desk = ChessMatch->GetDesk();

	// 가리키는 체스판 칸을 계산하고, 선택에 사용할 논리 커서를 해당 칸으로 옮긴다.
	if (Desk->ProjectRayToSquare(RayOrigin, RayDirection, Square))
	{
		HumanParticipant->SetCursorSquare(Square);
	}
}

void AChessPlayer::HandleInputMethodChanged(const ECommonInputType InputType)
{
	const bool bUsingPointerInput = InputType != ECommonInputType::Gamepad;
	HumanParticipant->SetUsingPointerInput(bUsingPointerInput);

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	PlayerController->bShowMouseCursor = bUsingPointerInput;
	if (bUsingPointerInput)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputMode);
	}
	else
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
	}
}

bool AChessPlayer::CanInteract_Implementation(APawn* Interactor)
{
	return IsHumanSeat()
		&& IsValid(Cast<AFirstPersonPlayer>(Interactor))
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

void AChessPlayer::PullChairIn()
{
	MoveChair(true);
}

void AChessPlayer::PullChairOut()
{
	MoveChair(false);
}

bool AChessPlayer::PullChairOutAndDetach(AActor* Occupant)
{
	if (AttachedSeatOccupant != Occupant)
	{
		return false;
	}

	SeatOccupantToDetachAfterMove = Occupant;
	PullChairOut();
	return true;
}

void AChessPlayer::AttachSeatOccupant(AActor* Occupant)
{
	AttachedSeatOccupant = Occupant;
	if (ACharacter* Character = Cast<ACharacter>(Occupant))
	{
		Character->GetCharacterMovement()->DisableMovement();
	}
	Occupant->AttachToComponent(SeatAnchor, FAttachmentTransformRules::KeepWorldTransform);
}

void AChessPlayer::DetachSeatOccupant(AActor* Occupant)
{
	if (AttachedSeatOccupant != Occupant)
	{
		return;
	}

	Occupant->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	if (ACharacter* Character = Cast<ACharacter>(Occupant))
	{
		Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
	AttachedSeatOccupant = nullptr;
}

void AChessPlayer::MoveChair(const bool bPullIn)
{
	const FVector TargetLocation = bPullIn ? ChairOutWorldLocation + GetActorTransform().TransformVectorNoScale(ChairPulledInLocalOffset) : ChairOutWorldLocation;
	bChairMoveTargetPulledIn = bPullIn;
	bChairMovePending = true;
	PositionTween->TweenToPosition(TargetLocation, ChairMoveDuration);
}

void AChessPlayer::UpdateChairMove()
{
	if (!bChairMovePending || PositionTween->IsPositionTweening())
	{
		return;
	}

	bChairMovePending = false;
	bChairPulledIn = bChairMoveTargetPulledIn;
	if (!bChairPulledIn && SeatOccupantToDetachAfterMove)
	{
		DetachSeatOccupant(SeatOccupantToDetachAfterMove);
		SeatOccupantToDetachAfterMove = nullptr;
	}
	OnChairMoveFinished.Broadcast(bChairPulledIn);
}
