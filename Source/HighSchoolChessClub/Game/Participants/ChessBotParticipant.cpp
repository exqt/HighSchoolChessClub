#include "ChessBotParticipant.h"

#include "ChessBotSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Game/ChessDesk.h"
#include "Game/ChessHandAnimation.h"
#include "Game/ChessMatchComponent.h"

void UChessBotParticipant::BeginTurn()
{
	Super::BeginTurn();

	FOnChessBotMoveReady OnCompleted;
	OnCompleted.BindDynamic(this, &UChessBotParticipant::OnMoveReady);
	PendingRequestId = Match->GetWorld()->GetGameInstance()->GetSubsystem<UChessBotSubsystem>()->RequestMove(
		Match->GetFen(),
		BotSettings,
		OnCompleted);
}

void UChessBotParticipant::EndTurn()
{
	PendingMoveId = INDEX_NONE;

	if (PendingRequestId.IsValid())
	{
		Match->GetWorld()->GetGameInstance()->GetSubsystem<UChessBotSubsystem>()->CancelRequest(PendingRequestId);
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

	IChessHandAnimation* HandAnimation = Cast<IChessHandAnimation>(GetPerformer());
	FChessMoveAnimationData MoveData;
	if (HandAnimation && Match->GetDesk()->MakeMoveAnimationData(Move, MovingPiece, MoveData))
	{
		MoveData.MoveId = ++LastMoveId;
		PendingMove = Move;
		PendingMoveId = MoveData.MoveId;
		HandAnimation->StartChessHandAnimation(MoveData);
		return;
	}

	Match->TrySubmitMove(this, Move);
}

void UChessBotParticipant::FinishChessMoveAnimation(const int32 MoveId)
{
	if (!bIsTurnActive || MoveId != PendingMoveId)
	{
		return;
	}

	const FChessCoreMove Move = PendingMove;
	PendingMoveId = INDEX_NONE;
	Match->TrySubmitMove(this, Move, EChessMoveVisualMode::Immediate);
}
