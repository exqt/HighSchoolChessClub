#include "Game/ChessMatchComponent.h"

#include "ChessGameState.h"
#include "Characters/NPC/NPCBase.h"
#include "Game/Participants/ChessBotParticipant.h"
#include "Game/ChessClockComponent.h"
#include "Game/ChessDesk.h"
#include "Game/Participants/ChessHumanParticipant.h"
#include "Game/Participants/ChessParticipant.h"

UChessMatchComponent::UChessMatchComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UChessMatchComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

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

void UChessMatchComponent::BeginPlay()
{
	Super::BeginPlay();
	ChessState = NewObject<UChessGameState>(this);
	GetChessClock()->OnTimeExpired.AddDynamic(this, &ThisClass::HandleTimeExpired);

	SetupInitialPosition();
	SetMatchState(EChessMatchState::WaitingForPlayers);
	RefreshParticipantState();
}

void UChessMatchComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetChessClock()->OnTimeExpired.RemoveDynamic(this, &ThisClass::HandleTimeExpired);
	GetChessClock()->StopClock();
	if (PlayerAParticipant)
	{
		PlayerAParticipant->EndTurn();
	}
	if (PlayerBParticipant)
	{
		PlayerBParticipant->EndTurn();
	}
	for (ANPCBase* NPCPerformer : GetNPCPerformers())
	{
		NPCPerformer->SetChessParticipant(nullptr);
	}
	Super::EndPlay(EndPlayReason);
}

AChessDesk* UChessMatchComponent::GetDesk() const
{
	return CastChecked<AChessDesk>(GetOwner());
}

UChessClockComponent* UChessMatchComponent::GetChessClock() const
{
	return GetDesk()->GetChessClock();
}

UChessParticipant* UChessMatchComponent::GetParticipant(const EChessPlayerPosition Position) const
{
	return Position == EChessPlayerPosition::PlayerA ? PlayerAParticipant : PlayerBParticipant;
}

AActor* UChessMatchComponent::GetPerformer(const EChessPlayerPosition Position) const
{
	const UChessParticipant* Participant = GetParticipant(Position);
	return Participant ? Participant->GetPerformer() : nullptr;
}

TArray<ANPCBase*> UChessMatchComponent::GetNPCPerformers() const
{
	TArray<ANPCBase*> NPCPerformers;
	if (ANPCBase* PlayerANPC = Cast<ANPCBase>(GetPerformer(EChessPlayerPosition::PlayerA)))
	{
		NPCPerformers.Add(PlayerANPC);
	}
	if (ANPCBase* PlayerBNPC = Cast<ANPCBase>(GetPerformer(EChessPlayerPosition::PlayerB)))
	{
		NPCPerformers.Add(PlayerBNPC);
	}
	return NPCPerformers;
}

UChessHumanParticipant* UChessMatchComponent::GetHumanParticipant(const EChessPlayerPosition Position) const
{
	return Cast<UChessHumanParticipant>(GetParticipant(Position));
}

UChessParticipant* UChessMatchComponent::RegisterParticipant(const EChessPlayerPosition Position, AActor* Performer, const TSubclassOf<UChessParticipant> ParticipantClass)
{
	TObjectPtr<UChessParticipant>& Participant = Position == EChessPlayerPosition::PlayerA ? PlayerAParticipant : PlayerBParticipant;

	if (Participant)
	{
		return Participant->GetPerformer() == Performer ? Participant.Get() : nullptr;
	}

	if (!ParticipantClass)
	{
		return nullptr;
	}

	Participant = NewObject<UChessParticipant>(this, ParticipantClass);
	Participant->Initialize(this, Position, Performer);
	if (ANPCBase* NPCPerformer = Cast<ANPCBase>(Performer))
	{
		NPCPerformer->SetChessParticipant(Participant);
	}
	RefreshParticipantState();
	OnParticipantRegistered.Broadcast(Participant);

	return Participant;
}

bool UChessMatchComponent::UnregisterParticipant(UChessParticipant* Participant)
{
	TObjectPtr<UChessParticipant>& RegisteredParticipant =
		Participant->GetPosition() == EChessPlayerPosition::PlayerA ? PlayerAParticipant : PlayerBParticipant;
	
	if (RegisteredParticipant != Participant)
	{
		return false;
	}

	Participant->EndTurn();
	if (ANPCBase* NPCPerformer = Cast<ANPCBase>(Participant->GetPerformer()))
	{
		NPCPerformer->SetChessParticipant(nullptr);
	}
	RegisteredParticipant = nullptr;
	if (MatchState == EChessMatchState::Playing)
	{
		FinishMatch();
	}
	else
	{
		RefreshParticipantState();
	}
	OnParticipantUnregistered.Broadcast(Participant);

	return true;
}

UChessParticipant* UChessMatchComponent::RegisterNPCParticipant(const EChessPlayerPosition Position, ANPCBase* NPCPerformer)
{
	return NPCPerformer ? RegisterParticipant(Position, NPCPerformer, NPCPerformer->GetChessParticipantClass()) : nullptr;
}

bool UChessMatchComponent::UnregisterNPCParticipant(const EChessPlayerPosition Position, ANPCBase* NPCPerformer)
{
	UChessParticipant* Participant = GetParticipant(Position);
	return Participant && Participant->GetPerformer() == NPCPerformer && UnregisterParticipant(Participant);
}

void UChessMatchComponent::RequestStartMatch()
{
	if (MatchState != EChessMatchState::MatchSetup || !PlayerAParticipant || !PlayerBParticipant)
	{
		return;
	}

	SetMatchState(EChessMatchState::Playing);
	BeginCurrentTurn();
}

void UChessMatchComponent::SetMatchSettings(const FChessMatchSettings& InSettings)
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
	GetDesk()->SetBoardFlipped(PlayerAColor == EChessCorePieceColor::White);

	GetChessClock()->ConfigureClock(MatchSettings.InitialTimeSeconds * 10, MatchSettings.IncrementSeconds * 10);
	GetChessClock()->ResetClock();
}

void UChessMatchComponent::NotifyHumanPlayerUnpossessed(UChessHumanParticipant* Participant)
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

bool UChessMatchComponent::TrySubmitMove(UChessParticipant* Participant, const FChessCoreMove& Move, const EChessMoveVisualMode VisualMode)
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
	GetDesk()->ApplyMoveToPieceActors(AppliedMove, MovingPiece, PieceAfterMove, VisualMode);
	OnBoardStateChanged.Broadcast(ChessState);

	Participant->EndTurn();
	GetChessClock()->ApplyIncrement(Participant->GetPosition());

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

bool UChessMatchComponent::TrySubmitMoveUci(UChessParticipant* Participant, const FString& UciMove)
{
	FChessCoreMove Move;
	return FindLegalMoveUci(UciMove, Move) && TrySubmitMove(Participant, Move);
}

bool UChessMatchComponent::FindLegalMoveUci(const FString& UciMove, FChessCoreMove& OutMove) const
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

void UChessMatchComponent::GetLegalMovesFrom(const FIntPoint Square, TArray<FChessCoreMove>& OutMoves) const
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

bool UChessMatchComponent::GetPieceAtSquare(const FIntPoint Square, FChessCorePiece& OutPiece) const
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

FString UChessMatchComponent::GetFen() const
{
	return ChessState->GetFen();
}

EChessCorePieceColor UChessMatchComponent::GetSideToMove() const
{
	return ChessState->GetSideToMove();
}

EChessCorePieceColor UChessMatchComponent::GetPlayerColor(const EChessPlayerPosition Position) const
{
	if (Position == EChessPlayerPosition::PlayerA)
	{
		return PlayerAColor;
	}
	return PlayerAColor == EChessCorePieceColor::White
		? EChessCorePieceColor::Black
		: EChessCorePieceColor::White;
}

EChessCorePieceColor UChessMatchComponent::GetHumanPlayerColor() const
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

void UChessMatchComponent::SetupInitialPosition()
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
	GetChessClock()->ResetClock();
	RebuildDeskFromState();
	OnBoardStateChanged.Broadcast(ChessState);
}

bool UChessMatchComponent::SetupPositionFromFen(const FString& Fen)
{
	if (MatchState != EChessMatchState::MatchSetup || !ChessState->SetFen(Fen))
	{
		return false;
	}

	GetChessClock()->ResetClock();
	RebuildDeskFromState();
	OnBoardStateChanged.Broadcast(ChessState);
	return true;
}

void UChessMatchComponent::HandleTimeExpired(const EChessPlayerPosition Position)
{
	FinishMatch();
}

void UChessMatchComponent::RefreshParticipantState()
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

void UChessMatchComponent::EnterMatchSetup()
{
	SetMatchState(EChessMatchState::MatchSetup);

	if (bUseDebugPosition)
	{
		SetupPositionFromFen(DebugPositionFen);
	}
}

void UChessMatchComponent::FinishMatch()
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
	GetChessClock()->StopClock();
	SetMatchState(EChessMatchState::Finished);
}

void UChessMatchComponent::SetMatchState(const EChessMatchState NewState)
{
	if (MatchState == NewState)
	{
		return;
	}

	MatchState = NewState;
	OnMatchStateChanged.Broadcast(NewState);
}

void UChessMatchComponent::BeginCurrentTurn()
{
	const EChessCorePieceColor SideToMove = ChessState->GetSideToMove();
	const EChessPlayerPosition ActivePosition = SideToMove == PlayerAColor ? EChessPlayerPosition::PlayerA : EChessPlayerPosition::PlayerB;
	UChessParticipant* ActiveParticipant = GetParticipantForColor(SideToMove);
	if (!ActiveParticipant)
	{
		FinishMatch();
		return;
	}

	if (GetChessClock()->IsClockRunning())
	{
		GetChessClock()->SetActivePlayer(ActivePosition);
	}
	else
	{
		GetChessClock()->StartClock(ActivePosition);
	}
	
	ActiveParticipant->BeginTurn();
}

void UChessMatchComponent::RebuildDeskFromState()
{
	TArray<FChessCorePiece> Pieces;
	ChessState->GetPieces(Pieces);
	GetDesk()->RebuildPieceActors(Pieces);
}

UChessParticipant* UChessMatchComponent::GetParticipantForColor(const EChessCorePieceColor Color) const
{
	return Color == PlayerAColor ? PlayerAParticipant : PlayerBParticipant;
}
