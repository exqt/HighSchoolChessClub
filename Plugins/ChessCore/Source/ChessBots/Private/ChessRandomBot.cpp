// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChessRandomBot.h"

#include <chrono>
#include <random>
#include <thread>

#ifdef check
#pragma push_macro("check")
#undef check
#define CHESSBOTS_RESTORE_CHECK_MACRO 1
#endif

THIRD_PARTY_INCLUDES_START
#include "chess.hpp"
THIRD_PARTY_INCLUDES_END

#ifdef CHESSBOTS_RESTORE_CHECK_MACRO
#pragma pop_macro("check")
#undef CHESSBOTS_RESTORE_CHECK_MACRO
#endif

FChessBotSearchResult FChessRandomBot::FindMove(const FChessBotSearchRequest& Request, const FChessBotCancellationToken& CancellationToken)
{
	const auto StartTime = std::chrono::steady_clock::now();
	FChessBotSearchResult Result;

	if (CancellationToken.IsCancellationRequested())
	{
		Result.Status = EChessBotSearchStatus::Cancelled;
		return Result;
	}

	chess::Board Board;
	if (!Board.setFen(Request.Fen))
	{
		Result.Status = EChessBotSearchStatus::InvalidPosition;
		return Result;
	}

	chess::Movelist Moves;
	chess::movegen::legalmoves(Moves, Board);
	Result.SearchedNodes = Moves.size();

	if (CancellationToken.IsCancellationRequested())
	{
		Result.Status = EChessBotSearchStatus::Cancelled;
	}
	else if (Moves.empty())
	{
		Result.Status = EChessBotSearchStatus::NoLegalMoves;
	}
	else
	{
		constexpr auto ThinkDelay = std::chrono::seconds(1);
		const auto ThinkEndTime = StartTime + ThinkDelay;
		while (std::chrono::steady_clock::now() < ThinkEndTime)
		{
			if (CancellationToken.IsCancellationRequested())
			{
				Result.Status = EChessBotSearchStatus::Cancelled;
				const auto EndTime = std::chrono::steady_clock::now();
				Result.ElapsedSeconds = std::chrono::duration<double>(EndTime - StartTime).count();
				return Result;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		std::mt19937_64 RandomGenerator(Request.RandomSeed);
		std::uniform_int_distribution<size_t> Distribution(0, Moves.size() - 1);
		const chess::Move SelectedMove = Moves[Distribution(RandomGenerator)];

		Result.Status = EChessBotSearchStatus::MoveFound;
		Result.UciMove = chess::uci::moveToUci(SelectedMove, Board.chess960());
	}

	const auto EndTime = std::chrono::steady_clock::now();
	Result.ElapsedSeconds = std::chrono::duration<double>(EndTime - StartTime).count();
	return Result;
}
