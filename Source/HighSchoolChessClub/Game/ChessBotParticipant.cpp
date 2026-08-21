#include "Game/ChessBotParticipant.h"

#include "ChessBotSubsystem.h"
#include "Engine/GameInstance.h"
#include "Game/ChessMatch.h"

void UChessBotParticipant::BeginTurn()
{
	Super::BeginTurn();

	FOnChessBotMoveReady OnCompleted;
	OnCompleted.BindDynamic(this, &UChessBotParticipant::OnMoveReady);
	PendingRequestId = Match->GetGameInstance()->GetSubsystem<UChessBotSubsystem>()->RequestMove(
		Match->GetFen(),
		BotSettings,
		OnCompleted);
}

void UChessBotParticipant::EndTurn()
{
	if (PendingRequestId.IsValid())
	{
		Match->GetGameInstance()->GetSubsystem<UChessBotSubsystem>()->CancelRequest(PendingRequestId);
		PendingRequestId.Invalidate();
	}
	Super::EndTurn();
}

void UChessBotParticipant::OnMoveReady(const FChessBotResult& Result)
{
	if (Result.RequestId != PendingRequestId)
	{
		return;
	}

	PendingRequestId.Invalidate();
	if (bIsTurnActive && Result.HasMove())
	{
		Match->TrySubmitMoveUci(this, Result.UciMove);
	}
}
