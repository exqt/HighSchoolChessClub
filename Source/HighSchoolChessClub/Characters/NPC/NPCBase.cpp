#include "Characters/NPC/NPCBase.h"

#include "Components/CapsuleComponent.h"
#include "Dialogue/CCDialogueSubsystem.h"
#include "Game/InteractionWidgetComponent.h"
#include "Game/Participants/ChessBotParticipant.h"

ANPCBase::ANPCBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = true;
	AutoPossessAI = EAutoPossessAI::PlacedInWorld;
	ChessParticipantClass = UChessBotParticipant::StaticClass();

	InteractionWidget = CreateDefaultSubobject<UInteractionWidgetComponent>(TEXT("InteractionWidgetComponent"));
	InteractionWidget->SetupAttachment(GetCapsuleComponent());
	InteractionWidget->SetRelativeLocation(FVector(30.0, 0.0, 30.0));
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

bool ANPCBase::CanInteract_Implementation(APawn*)
{
	return (NPCState == ENPCState::StandingIdle || NPCState == ENPCState::Texting) && !bIsDialogueActive;
}

FText ANPCBase::GetInteractionName_Implementation()
{
	return NSLOCTEXT("Interaction", "Talk", "말 걸기");
}

void ANPCBase::FinishChessMoveAnimation(const int32 MoveId)
{
	if (UChessBotParticipant* BotParticipant = Cast<UChessBotParticipant>(ActiveChessParticipant))
	{
		BotParticipant->FinishChessMoveAnimation(MoveId);
	}
}

void ANPCBase::HandleDialogueActiveChanged(bool bIsActive)
{
	bIsDialogueActive = bIsActive;
}
