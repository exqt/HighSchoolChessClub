#include "Game/ChessMatch.h"

#include "ChessGameState.h"
#include "Game/ChessClockComponent.h"
#include "Game/ChessDesk.h"
#include "Game/ChessHumanParticipant.h"
#include "Game/ChessParticipant.h"

AChessMatch::AChessMatch()
{
	PrimaryActorTick.bCanEverTick = true;
	ChessClock = CreateDefaultSubobject<UChessClockComponent>(TEXT("Chess Clock"));
	PlayerAParticipantClass = UChessHumanParticipant::StaticClass();
	PlayerBParticipantClass = UChessHumanParticipant::StaticClass();
}

void AChessMatch::BeginPlay()
{
	Super::BeginPlay();
	ChessState = NewObject<UChessGameState>(this);
	CreateParticipants();
	SetupInitialPosition();
}

void AChessMatch::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	PlayerAParticipant->EndTurn();
	PlayerBParticipant->EndTurn();
	Super::EndPlay(EndPlayReason);
}

UChessParticipant* AChessMatch::GetParticipant(const EChessPlayerPosition Position) const
{
	return Position == EChessPlayerPosition::PlayerA ? PlayerAParticipant : PlayerBParticipant;
}

UChessHumanParticipant* AChessMatch::GetHumanParticipant(const EChessPlayerPosition Position) const
{
	return Cast<UChessHumanParticipant>(GetParticipant(Position));
}

bool AChessMatch::TrySubmitMove(UChessParticipant* Participant, const FChessCoreMove& Move)
{
	if (Participant != GetParticipantForColor(ChessState->GetSideToMove()))
	{
		return false;
	}

	FChessCorePiece MovingPiece;
	if (!GetPieceAtSquare(FIntPoint(Move.From.File, Move.From.Rank), MovingPiece))
	{
		return false;
	}

	FChessCoreMove AppliedMove;
	if (!ChessState->TryMakeMove(Move, AppliedMove))
	{
		return false;
	}

	FChessCorePiece PieceAfterMove;
	const bool bHasPieceAfterMove = GetPieceAtSquare(
		FIntPoint(AppliedMove.To.File, AppliedMove.To.Rank),
		PieceAfterMove);

	if (Desk && (!bHasPieceAfterMove || !Desk->ApplyMoveToPieceActors(AppliedMove, MovingPiece, PieceAfterMove)))
	{
		RebuildDeskFromState();
	}

	Participant->EndTurn();
	OnMoveApplied(AppliedMove);
	BeginCurrentTurn();
	return true;
}

bool AChessMatch::TrySubmitMoveUci(UChessParticipant* Participant, const FString& UciMove)
{
	TArray<FChessCoreMove> LegalMoves;
	ChessState->GetLegalMoves(LegalMoves);
	const FChessCoreMove* Move = LegalMoves.FindByPredicate(
		[&UciMove](const FChessCoreMove& Candidate)
		{
			return Candidate.Uci == UciMove;
		});

	return Move && TrySubmitMove(Participant, *Move);
}

void AChessMatch::GetLegalMovesFrom(const FIntPoint Square, TArray<FChessCoreMove>& OutMoves) const
{
	TArray<FChessCoreMove> LegalMoves;
	ChessState->GetLegalMoves(LegalMoves);
	OutMoves.Reset();

	for (const FChessCoreMove& Move : LegalMoves)
	{
		if (Move.From.File == Square.X && Move.From.Rank == Square.Y)
		{
			OutMoves.Add(Move);
		}
	}
}

bool AChessMatch::GetPieceAtSquare(const FIntPoint Square, FChessCorePiece& OutPiece) const
{
	TArray<FChessCorePiece> Pieces;
	ChessState->GetPieces(Pieces);
	for (const FChessCorePiece& Piece : Pieces)
	{
		if (Piece.Square.File == Square.X && Piece.Square.Rank == Square.Y)
		{
			OutPiece = Piece;
			return true;
		}
	}
	return false;
}

FString AChessMatch::GetFen() const
{
	return ChessState->GetFen();
}

EChessCorePieceColor AChessMatch::GetSideToMove() const
{
	return ChessState->GetSideToMove();
}

EChessCorePieceColor AChessMatch::GetPlayerColor(const EChessPlayerPosition Position) const
{
	if (Position == EChessPlayerPosition::PlayerA)
	{
		return PlayerAColor;
	}
	return PlayerAColor == EChessCorePieceColor::White
		? EChessCorePieceColor::Black
		: EChessCorePieceColor::White;
}

void AChessMatch::SetupInitialPosition()
{
	PlayerAParticipant->EndTurn();
	PlayerBParticipant->EndTurn();
	ChessState->ResetToStartPosition();
	RebuildDeskFromState();
	BeginCurrentTurn();
}

void AChessMatch::CreateParticipants()
{
	UClass* PlayerAClass = PlayerAParticipantClass
		? PlayerAParticipantClass.Get()
		: UChessHumanParticipant::StaticClass();
	UClass* PlayerBClass = PlayerBParticipantClass
		? PlayerBParticipantClass.Get()
		: UChessHumanParticipant::StaticClass();

	PlayerAParticipant = NewObject<UChessParticipant>(this, PlayerAClass);
	PlayerBParticipant = NewObject<UChessParticipant>(this, PlayerBClass);
	PlayerAParticipant->Initialize(this, EChessPlayerPosition::PlayerA, PlayerAPerformer);
	PlayerBParticipant->Initialize(this, EChessPlayerPosition::PlayerB, PlayerBPerformer);
}

void AChessMatch::BeginCurrentTurn()
{
	GetParticipantForColor(ChessState->GetSideToMove())->BeginTurn();
}

void AChessMatch::RebuildDeskFromState()
{
	if (!Desk)
	{
		return;
	}

	TArray<FChessCorePiece> Pieces;
	ChessState->GetPieces(Pieces);
	Desk->RebuildPieceActors(Pieces);
}

UChessParticipant* AChessMatch::GetParticipantForColor(const EChessCorePieceColor Color) const
{
	return Color == PlayerAColor ? PlayerAParticipant : PlayerBParticipant;
}
