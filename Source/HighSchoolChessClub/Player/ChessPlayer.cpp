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
#include "Player/FirstPersonPlayer.h"

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
	APlayerController* PlayerController = Cast<APlayerController>(InExplorationPawn->GetController());
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
	AActor* PreviousViewTarget = PlayerController->GetViewTarget();
	AFirstPersonPlayer* PawnToRestore = ExplorationPawn;
	
	PlayerController->Possess(PawnToRestore);

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
	}
}

void AChessPlayer::EndPlayerView(APlayerController* PlayerController)
{
	if (ViewingController.Get() != PlayerController)
	{
		return;
	}

	ViewingController.Reset();
}

void AChessPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &AChessPlayer::LookInput);
	}
}

void AChessPlayer::LookInput(const FInputActionValue& Value)
{
	const FVector2D LookAxis = Value.Get<FVector2D>();
	AddControllerYawInput(LookAxis.X);
	AddControllerPitchInput(LookAxis.Y);
}

bool AChessPlayer::CanInteract_Implementation(APawn* Interactor)
{
	return IsValid(Cast<AFirstPersonPlayer>(Interactor)) && !IsValid(ExplorationPawn);
}

void AChessPlayer::Interact_Implementation(APawn* Interactor)
{
	if (AFirstPersonPlayer* Player = Cast<AFirstPersonPlayer>(Interactor))
	{
		Player->SitDown(this);
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
