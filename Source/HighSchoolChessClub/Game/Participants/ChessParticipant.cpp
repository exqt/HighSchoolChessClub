#include "ChessParticipant.h"

void UChessParticipant::Initialize(AChessMatch* InMatch, const EChessPlayerPosition InPosition, AActor* InPerformer) 
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
