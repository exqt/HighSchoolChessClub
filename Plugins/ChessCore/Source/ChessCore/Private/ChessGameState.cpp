// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChessGameState.h"

#ifdef check
#pragma push_macro("check")
#undef check
#define CHESSCORE_RESTORE_CHECK_MACRO 1
#endif

THIRD_PARTY_INCLUDES_START
#include "chess.hpp"
THIRD_PARTY_INCLUDES_END

#ifdef CHESSCORE_RESTORE_CHECK_MACRO
#pragma pop_macro("check")
#undef CHESSCORE_RESTORE_CHECK_MACRO
#endif

namespace
{
	constexpr const TCHAR* StartFen = TEXT("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

	chess::Square ToLibrarySquare(const FChessCoreSquare& Square)
	{
		return chess::Square{chess::File(Square.File), chess::Rank(Square.Rank)};
	}

	FChessCoreSquare FromLibrarySquare(chess::Square Square)
	{
		return FChessCoreSquare(static_cast<int32>(Square.file()), static_cast<int32>(Square.rank()));
	}

	FString ToUciSquare(const FChessCoreSquare& Square)
	{
		if (!Square.IsValid())
		{
			return FString();
		}

		const TCHAR FileChar = static_cast<TCHAR>('a' + Square.File);
		const TCHAR RankChar = static_cast<TCHAR>('1' + Square.Rank);
		return FString::Printf(TEXT("%c%c"), FileChar, RankChar);
	}

	EChessCorePieceType FromLibraryPieceType(chess::PieceType Type)
	{
		switch (Type.internal())
		{
		case chess::PieceType::PAWN:
			return EChessCorePieceType::Pawn;
		case chess::PieceType::KNIGHT:
			return EChessCorePieceType::Knight;
		case chess::PieceType::BISHOP:
			return EChessCorePieceType::Bishop;
		case chess::PieceType::ROOK:
			return EChessCorePieceType::Rook;
		case chess::PieceType::QUEEN:
			return EChessCorePieceType::Queen;
		case chess::PieceType::KING:
			return EChessCorePieceType::King;
		default:
			return EChessCorePieceType::None;
		}
	}

	EChessCorePieceColor FromLibraryColor(chess::Color Color)
	{
		switch (Color.internal())
		{
		case chess::Color::WHITE:
			return EChessCorePieceColor::White;
		case chess::Color::BLACK:
			return EChessCorePieceColor::Black;
		default:
			return EChessCorePieceColor::None;
		}
	}

	FChessCoreMove FromLibraryMove(const chess::Move& Move, const chess::Board& Board)
	{
		FChessCoreMove Result;
		Result.From = FromLibrarySquare(Move.from());
		Result.To = FromLibrarySquare(Move.to());
		Result.Promotion = Move.typeOf() == chess::Move::PROMOTION ? FromLibraryPieceType(Move.promotionType()) : EChessCorePieceType::None;
		Result.Uci = UTF8_TO_TCHAR(chess::uci::moveToUci(Move, Board.chess960()).c_str());
		return Result;
	}

	FString ToUciMove(const FChessCoreMove& Move)
	{
		if (!Move.Uci.IsEmpty())
		{
			return Move.Uci;
		}

		FString Uci = ToUciSquare(Move.From) + ToUciSquare(Move.To);
		switch (Move.Promotion)
		{
		case EChessCorePieceType::Knight:
			Uci += TEXT("n");
			break;
		case EChessCorePieceType::Bishop:
			Uci += TEXT("b");
			break;
		case EChessCorePieceType::Rook:
			Uci += TEXT("r");
			break;
		case EChessCorePieceType::Queen:
			Uci += TEXT("q");
			break;
		default:
			break;
		}

		return Uci;
	}

	chess::Move ParseUciMove(const chess::Board& Board, const FString& UciMove)
	{
		const FTCHARToUTF8 UciUtf8(*UciMove);
		return chess::uci::uciToMove(Board, std::string_view(UciUtf8.Get(), UciUtf8.Length()));
	}

	EChessCoreGameResult FromLibraryResult(chess::GameResult Result, chess::Color SideToMove)
	{
		switch (Result)
		{
		case chess::GameResult::WIN:
			return SideToMove == chess::Color::WHITE ? EChessCoreGameResult::WhiteWin : EChessCoreGameResult::BlackWin;
		case chess::GameResult::LOSE:
			return SideToMove == chess::Color::WHITE ? EChessCoreGameResult::BlackWin : EChessCoreGameResult::WhiteWin;
		case chess::GameResult::DRAW:
			return EChessCoreGameResult::Draw;
		default:
			return EChessCoreGameResult::None;
		}
	}

	EChessCoreGameResultReason FromLibraryReason(chess::GameResultReason Reason)
	{
		switch (Reason)
		{
		case chess::GameResultReason::CHECKMATE:
			return EChessCoreGameResultReason::Checkmate;
		case chess::GameResultReason::STALEMATE:
			return EChessCoreGameResultReason::Stalemate;
		case chess::GameResultReason::INSUFFICIENT_MATERIAL:
			return EChessCoreGameResultReason::InsufficientMaterial;
		case chess::GameResultReason::FIFTY_MOVE_RULE:
			return EChessCoreGameResultReason::FiftyMoveRule;
		case chess::GameResultReason::THREEFOLD_REPETITION:
			return EChessCoreGameResultReason::ThreefoldRepetition;
		default:
			return EChessCoreGameResultReason::None;
		}
	}
}

class FChessGameStateImpl
{
public:
	chess::Board Board;
};

UChessGameState::~UChessGameState()
{
	delete Impl;
	Impl = nullptr;
}

UChessGameState::UChessGameState()
	: Impl(new FChessGameStateImpl())
{
	ResetToStartPosition();
}

void UChessGameState::BeginDestroy()
{
	delete Impl;
	Impl = nullptr;
	Super::BeginDestroy();
}

void UChessGameState::ResetToStartPosition()
{
	SetFen(StartFen);
}

bool UChessGameState::SetFen(const FString& Fen)
{
	if (!Impl)
	{
		return false;
	}

	const FTCHARToUTF8 FenUtf8(*Fen);
	return Impl->Board.setFen(std::string_view(FenUtf8.Get(), FenUtf8.Length()));
}

FString UChessGameState::GetFen(bool bIncludeMoveCounters) const
{
	if (!Impl)
	{
		return FString();
	}

	return UTF8_TO_TCHAR(Impl->Board.getFen(bIncludeMoveCounters).c_str());
}

EChessCorePieceColor UChessGameState::GetSideToMove() const
{
	return Impl ? FromLibraryColor(Impl->Board.sideToMove()) : EChessCorePieceColor::None;
}

bool UChessGameState::IsInCheck() const
{
	return Impl && Impl->Board.inCheck();
}

bool UChessGameState::GetLegalMoves(TArray<FChessCoreMove>& OutMoves) const
{
	OutMoves.Reset();

	if (!Impl)
	{
		return false;
	}

	chess::Movelist Moves;
	chess::movegen::legalmoves(Moves, Impl->Board);
	OutMoves.Reserve(static_cast<int32>(Moves.size()));

	for (const chess::Move& Move : Moves)
	{
		OutMoves.Add(FromLibraryMove(Move, Impl->Board));
	}

	return true;
}

bool UChessGameState::TryMakeMove(const FChessCoreMove& Move, FChessCoreMove& AppliedMove)
{
	if (!Impl || !Move.From.IsValid() || !Move.To.IsValid())
	{
		return false;
	}

	const chess::Move Candidate = ParseUciMove(Impl->Board, ToUciMove(Move));
	if (!Impl->Board.isLegal(Candidate))
	{
		return false;
	}

	AppliedMove = FromLibraryMove(Candidate, Impl->Board);
	Impl->Board.makeMove(Candidate);
	return true;
}

bool UChessGameState::TryMakeMoveUci(const FString& UciMove, FChessCoreMove& AppliedMove)
{
	if (!Impl)
	{
		return false;
	}

	const chess::Move Move = ParseUciMove(Impl->Board, UciMove);
	if (Move == chess::Move::NO_MOVE || !Impl->Board.isLegal(Move))
	{
		return false;
	}

	AppliedMove = FromLibraryMove(Move, Impl->Board);
	Impl->Board.makeMove(Move);
	return true;
}

bool UChessGameState::GetPieces(TArray<FChessCorePiece>& OutPieces) const
{
	OutPieces.Reset();

	if (!Impl)
	{
		return false;
	}

	OutPieces.Reserve(32);
	for (int32 Rank = 0; Rank < 8; ++Rank)
	{
		for (int32 File = 0; File < 8; ++File)
		{
			const chess::Square LibrarySquare{chess::File(File), chess::Rank(Rank)};
			const chess::Piece Piece = Impl->Board.at<chess::Piece>(LibrarySquare);
			const EChessCorePieceType Type = FromLibraryPieceType(Piece.type());
			if (Type == EChessCorePieceType::None)
			{
				continue;
			}

			FChessCorePiece OutPiece;
			OutPiece.Square = FChessCoreSquare(File, Rank);
			OutPiece.Type = Type;
			OutPiece.Color = FromLibraryColor(Piece.color());
			OutPieces.Add(OutPiece);
		}
	}

	return true;
}

FChessCoreGameStatus UChessGameState::GetGameStatus() const
{
	FChessCoreGameStatus Status;

	if (!Impl)
	{
		return Status;
	}

	Status.bInCheck = Impl->Board.inCheck();
	const auto GameOver = Impl->Board.isGameOver();
	Status.Reason = FromLibraryReason(GameOver.first);
	Status.Result = FromLibraryResult(GameOver.second, Impl->Board.sideToMove());
	return Status;
}
