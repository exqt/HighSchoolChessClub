#include "Characters/NPC/NPCBase.h"

#include "Blueprint/UserWidget.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Dialogue/CCDialogueSubsystem.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ANPCBase::ANPCBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = true;
	AutoPossessAI = EAutoPossessAI::PlacedInWorld;

	InteractionWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionWidgetComponent"));
	InteractionWidget->SetupAttachment(GetCapsuleComponent());
	InteractionWidget->SetWidgetSpace(EWidgetSpace::Screen);
	InteractionWidget->SetDrawSize(FVector2D(310.0, 78.0));
	InteractionWidget->SetPivot(FVector2D(0.5, 0.5));
	InteractionWidget->SetRelativeLocation(FVector(30.0, 0.0, 30.0));
	InteractionWidget->SetRelativeScale3D(FVector(0.2));
	InteractionWidget->SetBlendMode(EWidgetBlendMode::Masked);
	InteractionWidget->SetTickWhenOffscreen(false);
}

void ANPCBase::BeginPlay()
{
	Super::BeginPlay();

	UCCDialogueSubsystem* DialogueSubsystem = UCCDialogueSubsystem::Get(this);
	DialogueSubsystem->OnDialogueActiveChanged.AddUniqueDynamic(
		this,
		&ThisClass::HandleDialogueActiveChanged
	);
}

void ANPCBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UCCDialogueSubsystem* DialogueSubsystem = UCCDialogueSubsystem::Get(this);
	DialogueSubsystem->OnDialogueActiveChanged.RemoveAll(this);

	Super::EndPlay(EndPlayReason);
}

void ANPCBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	CheckInteractionWidgetVisibilityDistance();
}

bool ANPCBase::CanInteract_Implementation(APawn*)
{
	return !bIsDialogueActive;
}

FText ANPCBase::GetInteractionName_Implementation()
{
	return NSLOCTEXT("Interaction", "Interact", "Interact");
}

void ANPCBase::HandleDialogueActiveChanged(bool bIsActive)
{
	bIsDialogueActive = bIsActive;
}

void ANPCBase::CheckInteractionWidgetVisibilityDistance() const
{
	if (bIsDialogueActive)
	{
		InteractionWidget->SetVisibility(false);
		return;
	}
	
	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	const float Dist = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
	
	InteractionWidget->SetVisibility(Dist < InteractionWidgetVisibilityDistance);
}
