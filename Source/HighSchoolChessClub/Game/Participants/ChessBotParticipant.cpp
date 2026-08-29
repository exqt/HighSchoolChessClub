#include "ChessBotParticipant.h"

#include "ChessBotSubsystem.h"
#include "Characters/NPC/NPCBase.h"
#include "Engine/GameInstance.h"
#include "Game/ChessDesk.h"
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
	ANPCBase* NPCPerformer = Cast<ANPCBase>(GetPerformer());
	if (NPCPerformer)
	{
		NPCPerformer->OnChessMoveAnimationFinished.RemoveAll(this);
	}
	PendingMoveId = INDEX_NONE;

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
	if (!bIsTurnActive || !Result.HasMove())
	{
		return;
	}

	FChessCoreMove Move;
	if (!Match->FindLegalMoveUci(Result.UciMove, Move))
	{
		return;
	}

	FChessCorePiece MovingPiece;
	Match->GetPieceAtSquare(FIntPoint(Move.From.File, Move.From.Rank), MovingPiece);

	ANPCBase* NPCPerformer = Cast<ANPCBase>(GetPerformer());
	FChessMoveAnimationData MoveData;
	if (NPCPerformer && Match->GetDesk()->MakeMoveAnimationData(Move, MovingPiece, MoveData))
	{
		MoveData.MoveId = ++LastMoveId;
		PendingMove = Move;
		PendingMoveId = MoveData.MoveId;
		NPCPerformer->OnChessMoveAnimationFinished.RemoveAll(this);
		NPCPerformer->OnChessMoveAnimationFinished.AddUObject(this, &ThisClass::HandleChessMoveAnimationFinished);
		NPCPerformer->StartChessMoveAnimation(MoveData);
		return;
	}

	Match->TrySubmitMove(this, Move);
}

void UChessBotParticipant::HandleChessMoveAnimationFinished(const int32 MoveId)
{
	if (!bIsTurnActive || MoveId != PendingMoveId)
	{
		return;
	}

	ANPCBase* NPCPerformer = Cast<ANPCBase>(GetPerformer());
	if (NPCPerformer)
	{
		NPCPerformer->OnChessMoveAnimationFinished.RemoveAll(this);
	}

	const FChessCoreMove Move = PendingMove;
	PendingMoveId = INDEX_NONE;
	Match->TrySubmitMove(this, Move, EChessMoveVisualMode::Immediate);
}
