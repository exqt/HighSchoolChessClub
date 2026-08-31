#include "ChessHumanParticipant.h"

#include "Game/ChessDesk.h"
#include "Game/ChessMatchComponent.h"

void UChessHumanParticipant::BeginTurn()
{
	Super::BeginTurn();
	RefreshCursorVisibility();
}

void UChessHumanParticipant::EndTurn()
{
	CancelSelection();
	Super::EndTurn();
	RefreshCursorVisibility();
}

bool UChessHumanParticipant::MoveCursor(const FIntPoint Delta)
{
	return CanAcceptInput() && Match->GetDesk()->MoveCursor(Delta);
}

bool UChessHumanParticipant::SetCursorSquare(const FIntPoint Square)
{
	return CanAcceptInput() && Match->GetDesk()->SetCursorSquare(Square);
}

bool UChessHumanParticipant::SelectCurrentSquare()
{
	if (!CanAcceptInput())
	{
		return false;
	}

	const FIntPoint CursorSquare = Match->GetDesk()->GetCursorSquare();
	if (!bHasSelectedSquare)
	{
		return SelectPieceAtCursor();
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
		const FChessCoreMove Move = *MoveToApply;
		CancelSelection();
		return Match->TrySubmitMove(this, Move);
	}

	FChessCorePiece Piece;
	if (Match->GetPieceAtSquare(CursorSquare, Piece) && Piece.Color == Match->GetPlayerColor(Position))
	{
		CancelSelection();
		return SelectPieceAtCursor();
	}

	return false;
}

void UChessHumanParticipant::CancelSelection()
{
	if (!bHasSelectedSquare)
	{
		return;
	}

	Match->GetDesk()->ClearPieceSelection(SelectedSquare);
	bHasSelectedSquare = false;
	SelectedSquare = FIntPoint::ZeroValue;
	SelectedLegalMoves.Reset();
}

void UChessHumanParticipant::SetUsingPointerInput(const bool bInUsingPointerInput)
{
	bUsingPointerInput = bInUsingPointerInput;
	RefreshCursorVisibility();
}

bool UChessHumanParticipant::CanAcceptInput() const
{
	return bIsTurnActive && Match->GetDesk();
}

bool UChessHumanParticipant::SelectPieceAtCursor()
{
	const FIntPoint CursorSquare = Match->GetDesk()->GetCursorSquare();
	FChessCorePiece Piece;
	if (!Match->GetPieceAtSquare(CursorSquare, Piece) || Piece.Color != Match->GetPlayerColor(Position))
	{
		return false;
	}

	Match->GetLegalMovesFrom(CursorSquare, SelectedLegalMoves);
	if (SelectedLegalMoves.IsEmpty())
	{
		return false;
	}

	bHasSelectedSquare = true;
	SelectedSquare = CursorSquare;
	Match->GetDesk()->ShowPieceSelection(SelectedSquare, GetLegalDestinationSquares());
	return true;
}

TArray<FIntPoint> UChessHumanParticipant::GetLegalDestinationSquares() const
{
	TArray<FIntPoint> Destinations;
	Destinations.Reserve(SelectedLegalMoves.Num());
	for (const FChessCoreMove& Move : SelectedLegalMoves)
	{
		Destinations.AddUnique(FIntPoint(Move.To.File, Move.To.Rank));
	}
	return Destinations;
}

void UChessHumanParticipant::RefreshCursorVisibility() const
{
	if (Match->GetDesk())
	{
		Match->GetDesk()->SetCursorVisible(CanAcceptInput() && !bUsingPointerInput);
	}
}
