#include "Game/ChessMatch.h"

#include "ChessGameState.h"
#include "Game/ChessDesk.h"

AChessMatch::AChessMatch()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AChessMatch::BeginPlay()
{
	Super::BeginPlay();
	ChessState = NewObject<UChessGameState>(this);
	SetupInitialPosition();
}

bool AChessMatch::MoveCursor(const FIntPoint Delta, const EChessPlayerPosition PlayerPosition)
{
	if (!Desk || !CanPlayerControl(PlayerPosition))
	{
		return false;
	}
	return Desk->MoveCursor(Delta);
}

bool AChessMatch::SelectCurrentSquare(const EChessPlayerPosition PlayerPosition)
{
	if (!Desk || !CanPlayerControl(PlayerPosition))
	{
		return false;
	}

	const EChessCorePieceColor PlayerColor = GetPlayerColor(PlayerPosition);
	if (PlayerColor == EChessCorePieceColor::None || ChessState->GetSideToMove() != PlayerColor)
	{
		return false;
	}

	const FIntPoint CursorSquare = Desk->GetCursorSquare();
	if (!bHasSelectedSquare)
	{
		return SelectPieceAtCursor(PlayerColor);
	}

	if (CursorSquare == SelectedSquare)
	{
		CancelSelection();
		return true;
	}

	const FChessCoreMove* MoveToApply = nullptr;
	for (const FChessCoreMove& Move : SelectedLegalMoves)
	{
		if (Move.To.File != CursorSquare.X || Move.To.Rank != CursorSquare.Y)
		{
			continue;
		}

		if (!MoveToApply || Move.Promotion == EChessCorePieceType::Queen)
		{
			MoveToApply = &Move;
		}
	}

	if (MoveToApply)
	{
		FChessCorePiece MovingPiece;
		if (!GetPieceAtSquare(SelectedSquare, MovingPiece))
		{
			return false;
		}

		FChessCoreMove AppliedMove;
		if (!ChessState->TryMakeMove(*MoveToApply, AppliedMove))
		{
			return false;
		}

		FChessCorePiece PieceAfterMove;
		const bool bHasPieceAfterMove = GetPieceAtSquare(CursorSquare, PieceAfterMove);
		CancelSelection();
		if (!bHasPieceAfterMove || !Desk->ApplyMoveToPieceActors(AppliedMove, MovingPiece, PieceAfterMove))
		{
			RebuildDeskFromState();
		}

		RefreshCursorVisibility();
		OnMoveApplied(AppliedMove);
		return true;
	}

	FChessCorePiece Piece;
	if (GetPieceAtSquare(CursorSquare, Piece) && Piece.Color == PlayerColor)
	{
		CancelSelection();
		return SelectPieceAtCursor(PlayerColor);
	}
	return false;
}

void AChessMatch::CancelSelection()
{
	if (!bHasSelectedSquare)
	{
		return;
	}

	if (Desk)
	{
		Desk->ClearPieceSelection(SelectedSquare);
	}
	bHasSelectedSquare = false;
	SelectedSquare = FIntPoint::ZeroValue;
	SelectedLegalMoves.Reset();
}

void AChessMatch::BeginPlayerControl(const EChessPlayerPosition PlayerPosition)
{
	ActivePlayerPosition = PlayerPosition;
	bHasActivePlayer = true;
	RefreshCursorVisibility();
}

void AChessMatch::EndPlayerControl(const EChessPlayerPosition PlayerPosition)
{
	if (!bHasActivePlayer || ActivePlayerPosition != PlayerPosition)
	{
		return;
	}
	bHasActivePlayer = false;
	CancelSelection();
	RefreshCursorVisibility();
}

bool AChessMatch::CanPlayerControl(const EChessPlayerPosition PlayerPosition) const
{
	return bHasActivePlayer
		&& ActivePlayerPosition == PlayerPosition
		&& ChessState
		&& ChessState->GetSideToMove() == GetPlayerColor(PlayerPosition);
}

TArray<FIntPoint> AChessMatch::GetLegalDestinationSquares() const
{
	TArray<FIntPoint> Destinations;
	Destinations.Reserve(SelectedLegalMoves.Num());
	for (const FChessCoreMove& Move : SelectedLegalMoves)
	{
		Destinations.AddUnique(FIntPoint(Move.To.File, Move.To.Rank));
	}
	return Destinations;
}

EChessCorePieceColor AChessMatch::GetSideToMove() const
{
	return ChessState ? ChessState->GetSideToMove() : EChessCorePieceColor::None;
}

EChessCorePieceColor AChessMatch::GetPlayerColor(const EChessPlayerPosition PlayerPosition) const
{
	if (PlayerPosition == EChessPlayerPosition::PlayerA)
	{
		return PlayerAColor;
	}
	return PlayerAColor == EChessCorePieceColor::White ? EChessCorePieceColor::Black : EChessCorePieceColor::White;
}

void AChessMatch::SetupInitialPosition()
{
	if (!ChessState)
	{
		ChessState = NewObject<UChessGameState>(this);
	}
	ChessState->ResetToStartPosition();
	CancelSelection();
	RebuildDeskFromState();
	RefreshCursorVisibility();
}

bool AChessMatch::SelectPieceAtCursor(const EChessCorePieceColor PlayerColor)
{
	const FIntPoint CursorSquare = Desk->GetCursorSquare();
	FChessCorePiece Piece;
	if (!GetPieceAtSquare(CursorSquare, Piece) || Piece.Color != PlayerColor)
	{
		return false;
	}

	TArray<FChessCoreMove> LegalMoves;
	ChessState->GetLegalMoves(LegalMoves);
	SelectedLegalMoves.Reset();
	for (const FChessCoreMove& Move : LegalMoves)
	{
		if (Move.From.File == CursorSquare.X && Move.From.Rank == CursorSquare.Y)
		{
			SelectedLegalMoves.Add(Move);
		}
	}

	if (SelectedLegalMoves.IsEmpty())
	{
		return false;
	}
	bHasSelectedSquare = true;
	SelectedSquare = CursorSquare;
	Desk->ShowPieceSelection(SelectedSquare, GetLegalDestinationSquares());
	return true;
}

bool AChessMatch::GetPieceAtSquare(const FIntPoint Square, FChessCorePiece& OutPiece) const
{
	TArray<FChessCorePiece> Pieces;
	if (!ChessState || !ChessState->GetPieces(Pieces))
	{
		return false;
	}
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

void AChessMatch::RebuildDeskFromState()
{
	if (!Desk || !ChessState)
	{
		return;
	}
	TArray<FChessCorePiece> Pieces;
	ChessState->GetPieces(Pieces);
	Desk->RebuildPieceActors(Pieces);
}

void AChessMatch::RefreshCursorVisibility()
{
	if (Desk)
	{
		Desk->SetCursorVisible(bHasActivePlayer && CanPlayerControl(ActivePlayerPosition));
	}
}
