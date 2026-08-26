#include "Characters/NPC/NPCBase.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CapsuleComponent.h"
#include "Components/TextBlock.h"
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
	UpdateInteractionWidgetText();
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
	return NPCState == ENPCState::StandingIdle && !bIsDialogueActive;
}

FText ANPCBase::GetInteractionName_Implementation()
{
	return NSLOCTEXT("Interaction", "Talk", "말 걸기");
}

void ANPCBase::HandleDialogueActiveChanged(bool bIsActive)
{
	bIsDialogueActive = bIsActive;
}

void ANPCBase::UpdateInteractionWidgetText()
{
	InteractionWidget->InitWidget();
	UUserWidget* UserWidget = InteractionWidget->GetUserWidgetObject();
	if (!UserWidget)
	{
		return;
	}

	TArray<UWidget*> Widgets;
	UserWidget->WidgetTree->GetAllWidgets(Widgets);
	for (UWidget* Widget : Widgets)
	{
		if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
		{
			TextBlock->SetText(IInteractable::Execute_GetInteractionName(this));
			return;
		}
	}
}

void ANPCBase::CheckInteractionWidgetVisibilityDistance() const
{
	if (NPCState != ENPCState::StandingIdle || bIsDialogueActive)
	{
		InteractionWidget->SetVisibility(false);
		return;
	}
	
	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	const float Dist = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
	
	InteractionWidget->SetVisibility(Dist < InteractionWidgetVisibilityDistance);
}
