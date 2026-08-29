#include "Game/ChessMatch.h"

#include "ChessGameState.h"
#include "Game/Participants/ChessBotParticipant.h"
#include "Game/ChessClockComponent.h"
#include "Game/ChessDesk.h"
#include "Game/Participants/ChessHumanParticipant.h"
#include "Game/Participants/ChessParticipant.h"

AChessMatch::AChessMatch()
{
	PrimaryActorTick.bCanEverTick = true;
	ChessClock = CreateDefaultSubobject<UChessClockComponent>(TEXT("Chess Clock"));
	PlayerAParticipantClass = UChessHumanParticipant::StaticClass();
	PlayerBParticipantClass = UChessBotParticipant::StaticClass();
}

void AChessMatch::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (MatchState != EChessMatchState::Playing || !ChessState)
	{
		return;
	}
	
	// TODO: Event 기반으로 처리하기
	if (ChessState->GetGameStatus().Result != EChessCoreGameResult::None)
	{
		FinishMatch();
		return;
	}
}

void AChessMatch::BeginPlay()
{
	Super::BeginPlay();
	ChessState = NewObject<UChessGameState>(this);
	ChessClock->OnTimeExpired.AddDynamic(this, &ThisClass::HandleTimeExpired);

	SetupInitialPosition();
	SetMatchState(EChessMatchState::WaitingForPlayers);
	RegisterConfiguredParticipants();
	RefreshParticipantState();
}

void AChessMatch::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ChessClock->OnTimeExpired.RemoveDynamic(this, &ThisClass::HandleTimeExpired);
	ChessClock->StopClock();
	if (PlayerAParticipant)
	{
		PlayerAParticipant->EndTurn();
	}
	if (PlayerBParticipant)
	{
		PlayerBParticipant->EndTurn();
	}
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

UChessParticipant* AChessMatch::RegisterParticipant(const EChessPlayerPosition Position, AActor* Performer)
{
	TObjectPtr<UChessParticipant>& Participant = Position == EChessPlayerPosition::PlayerA ? PlayerAParticipant : PlayerBParticipant;

	if (Participant)
	{
		return Participant->GetPerformer() == Performer ? Participant.Get() : nullptr;
	}

	const TSubclassOf<UChessParticipant> ParticipantClass = Position == EChessPlayerPosition::PlayerA ? PlayerAParticipantClass : PlayerBParticipantClass;
	UClass* RequiredClass = Position == EChessPlayerPosition::PlayerA ? UChessHumanParticipant::StaticClass() : UChessBotParticipant::StaticClass();
	UClass* ClassToCreate = ParticipantClass && ParticipantClass->IsChildOf(RequiredClass) ? ParticipantClass.Get() : RequiredClass;

	Participant = NewObject<UChessParticipant>(this, ClassToCreate);
	Participant->Initialize(this, Position, Performer);
	RefreshParticipantState();

	return Participant;
}

bool AChessMatch::UnregisterParticipant(UChessParticipant* Participant)
{
	TObjectPtr<UChessParticipant>& RegisteredParticipant =
		Participant->GetPosition() == EChessPlayerPosition::PlayerA ? PlayerAParticipant : PlayerBParticipant;
	
	if (RegisteredParticipant != Participant)
	{
		return false;
	}

	Participant->EndTurn();
	RegisteredParticipant = nullptr;
	RefreshParticipantState();

	return true;
}

void AChessMatch::RequestStartMatch()
{
	SetMatchState(EChessMatchState::Playing);
	BeginCurrentTurn();
}

void AChessMatch::SetMatchSettings(const FChessMatchSettings& InSettings)
{
	MatchSettings = InSettings;

	switch (MatchSettings.ColorSelection)
	{
	case EChessPlayerColor::White:
		PlayerAColor = EChessCorePieceColor::White;
		break;
	case EChessPlayerColor::Black:
		PlayerAColor = EChessCorePieceColor::Black;
		break;
	case EChessPlayerColor::Random:
		PlayerAColor = FMath::RandBool() ? EChessCorePieceColor::White : EChessCorePieceColor::Black;
		break;
	}

	ChessClock->ConfigureClock(MatchSettings.InitialTimeSeconds * 10, MatchSettings.IncrementSeconds * 10);
	ChessClock->ResetClock();
}

void AChessMatch::NotifyHumanPlayerUnpossessed(UChessHumanParticipant* Participant)
{
	if (!Participant || GetParticipant(Participant->GetPosition()) != Participant)
	{
		return;
	}

	if (MatchState == EChessMatchState::MatchSetup)
	{
		SetMatchState(EChessMatchState::WaitingForPlayers);
	}
	else if (MatchState == EChessMatchState::Playing)
	{
		FinishMatch();
	}
}

bool AChessMatch::TrySubmitMove(UChessParticipant* Participant, const FChessCoreMove& Move, const EChessMoveVisualMode VisualMode)
{
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
	GetPieceAtSquare(FIntPoint(AppliedMove.To.File, AppliedMove.To.Rank), PieceAfterMove);
	Desk->ApplyMoveToPieceActors(AppliedMove, MovingPiece, PieceAfterMove, VisualMode);
	OnBoardStateChanged.Broadcast(ChessState);

	Participant->EndTurn();
	ChessClock->ApplyIncrement(Participant->GetPosition());

	if (ChessState->GetGameStatus().Result != EChessCoreGameResult::None)
	{
		FinishMatch();
	}
	else
	{
		BeginCurrentTurn();
	}
	
	return true;
}

bool AChessMatch::TrySubmitMoveUci(UChessParticipant* Participant, const FString& UciMove)
{
	FChessCoreMove Move;
	return FindLegalMoveUci(UciMove, Move) && TrySubmitMove(Participant, Move);
}

bool AChessMatch::FindLegalMoveUci(const FString& UciMove, FChessCoreMove& OutMove) const
{
	TArray<FChessCoreMove> LegalMoves;
	ChessState->GetLegalMoves(LegalMoves);
	const FChessCoreMove* Move = LegalMoves.FindByPredicate(
		[&UciMove](const FChessCoreMove& Candidate)
		{
			return Candidate.Uci == UciMove;
		});

	if (!Move)
	{
		return false;
	}

	OutMove = *Move;
	return true;
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

EChessCorePieceColor AChessMatch::GetHumanPlayerColor() const
{
	if (GetHumanParticipant(EChessPlayerPosition::PlayerA))
	{
		return GetPlayerColor(EChessPlayerPosition::PlayerA);
	}
	if (GetHumanParticipant(EChessPlayerPosition::PlayerB))
	{
		return GetPlayerColor(EChessPlayerPosition::PlayerB);
	}
	return EChessCorePieceColor::None;
}

void AChessMatch::SetupInitialPosition()
{
	if (PlayerAParticipant)
	{
		PlayerAParticipant->EndTurn();
	}
	if (PlayerBParticipant)
	{
		PlayerBParticipant->EndTurn();
	}
	ChessState->ResetToStartPosition();
	ChessClock->ResetClock();
	RebuildDeskFromState();
	OnBoardStateChanged.Broadcast(ChessState);
}

bool AChessMatch::SetupPositionFromFen(const FString& Fen)
{
	if (MatchState != EChessMatchState::MatchSetup || !ChessState->SetFen(Fen))
	{
		return false;
	}

	ChessClock->ResetClock();
	RebuildDeskFromState();
	OnBoardStateChanged.Broadcast(ChessState);
	return true;
}

void AChessMatch::HandleTimeExpired(const EChessPlayerPosition Position)
{
	FinishMatch();
}

void AChessMatch::RegisterConfiguredParticipants()
{
	if (PlayerAPerformer)
	{
		RegisterParticipant(EChessPlayerPosition::PlayerA, PlayerAPerformer);
	}
	RegisterParticipant(EChessPlayerPosition::PlayerB, PlayerBPerformer);
}

void AChessMatch::RefreshParticipantState()
{
	if (!ChessState || MatchState == EChessMatchState::Playing || MatchState == EChessMatchState::Finished)
	{
		return;
	}

	if (!PlayerAParticipant || !PlayerBParticipant)
	{
		SetMatchState(EChessMatchState::WaitingForPlayers);
	}
}

void AChessMatch::EnterMatchSetup()
{
	SetMatchState(EChessMatchState::MatchSetup);

	if (bUseDebugPosition)
	{
		SetupPositionFromFen(DebugPositionFen);
	}
}

void AChessMatch::FinishMatch()
{
	if (MatchState == EChessMatchState::Finished)
	{
		return;
	}

	if (PlayerAParticipant)
	{
		PlayerAParticipant->EndTurn();
	}
	if (PlayerBParticipant)
	{
		PlayerBParticipant->EndTurn();
	}
	ChessClock->StopClock();
	SetMatchState(EChessMatchState::Finished);
}

void AChessMatch::SetMatchState(const EChessMatchState NewState)
{
	if (MatchState == NewState)
	{
		return;
	}

	MatchState = NewState;
	OnMatchStateChanged.Broadcast(NewState);
}

void AChessMatch::BeginCurrentTurn()
{
	const EChessPlayerPosition ActivePosition = 
		ChessState->GetSideToMove() == PlayerAColor ? EChessPlayerPosition::PlayerA : EChessPlayerPosition::PlayerB;
	
	if (ChessClock->IsClockRunning())
	{
		ChessClock->SetActivePlayer(ActivePosition);
	}
	else
	{
		ChessClock->StartClock(ActivePosition);
	}
	
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
