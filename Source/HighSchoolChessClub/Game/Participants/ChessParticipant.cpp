#include "ChessParticipant.h"

#include "Game/ChessMatchComponent.h"

void UChessParticipant::Initialize(UChessMatchComponent* InMatch, const EChessPlayerPosition InPosition, AActor* InPerformer)
{
	Match = InMatch;
	Position = InPosition;
	Performer = InPerformer;
}

void UChessParticipant::BeginTurn()
{
	bIsTurnActive = true;
}

void UChessParticipant::EndTurn()
{
	bIsTurnActive = false;
}

bool UChessParticipant::IsWinner(const EChessCoreGameResult GameResult) const
{
	const EChessCorePieceColor PlayerColor = Match->GetPlayerColor(Position);
	return (GameResult == EChessCoreGameResult::WhiteWin && PlayerColor == EChessCorePieceColor::White) || (GameResult == EChessCoreGameResult::BlackWin && PlayerColor == EChessCorePieceColor::Black);
}
