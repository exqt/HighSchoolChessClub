// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChessMctsBot.h"

#include "ChessBotLog.h"
#include "ChessOpeningTree.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <limits>
#include <memory>
#include <random>
#include <utility>
#include <vector>

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

namespace
{
	struct FMctsNode
	{
		FMctsNode(FMctsNode* InParent, const chess::Move InMove, const chess::Board& Board)
			: Parent(InParent)
			, MoveFromParent(InMove)
		{
			if (Parent && Board.isGameOver().second != chess::GameResult::NONE)
			{
				return;
			}

			chess::Movelist LegalMoves;
			chess::movegen::legalmoves(LegalMoves, Board);
			UnexpandedMoves.assign(LegalMoves.begin(), LegalMoves.end());
		}

		FMctsNode* Parent = nullptr;
		chess::Move MoveFromParent = chess::Move::NO_MOVE;
		std::vector<std::unique_ptr<FMctsNode>> Children;
		std::vector<chess::Move> UnexpandedMoves;
		uint64 Visits = 0;
		double ValueSum = 0.0;
	};

	double EvaluatePosition(const chess::Board& Board, const chess::Color RootColor, const FChessMctsSearchSettings& Settings)
	{
		const chess::GameResult GameResult = Board.isGameOver().second;
		if (GameResult == chess::GameResult::DRAW)
		{
			return 0.0;
		}
		if (GameResult == chess::GameResult::LOSE)
		{
			return Board.sideToMove() == RootColor ? -1.0 : 1.0;
		}
		if (GameResult == chess::GameResult::WIN)
		{
			return Board.sideToMove() == RootColor ? 1.0 : -1.0;
		}

		const std::array<std::pair<chess::PieceType::underlying, double>, 5> PieceValues = {{{chess::PieceType::PAWN, Settings.PawnValue}, {chess::PieceType::KNIGHT, Settings.KnightValue}, {chess::PieceType::BISHOP, Settings.BishopValue}, {chess::PieceType::ROOK, Settings.RookValue}, {chess::PieceType::QUEEN, Settings.QueenValue}}};
		double MaterialScore = 0.0;
		for (const auto& [PieceTypeValue, PieceValue] : PieceValues)
		{
			const chess::PieceType PieceType(PieceTypeValue);
			MaterialScore += PieceValue * (Board.pieces(PieceType, RootColor).count() - Board.pieces(PieceType, ~RootColor).count());
		}
		return std::tanh(MaterialScore / Settings.MaterialScoreScale);
	}

	FMctsNode* SelectChild(FMctsNode& Node, const bool bRootTurn, const double ExplorationConstant)
	{
		FMctsNode* BestChild = nullptr;
		double BestScore = -std::numeric_limits<double>::infinity();
		const double LogParentVisits = std::log(static_cast<double>(std::max<uint64>(1, Node.Visits)));
		for (const std::unique_ptr<FMctsNode>& Child : Node.Children)
		{
			const double MeanValue = Child->ValueSum / static_cast<double>(Child->Visits);
			const double Exploration = ExplorationConstant * std::sqrt(LogParentVisits / static_cast<double>(Child->Visits));
			const double Score = (bRootTurn ? MeanValue : -MeanValue) + Exploration;
			if (Score > BestScore)
			{
				BestScore = Score;
				BestChild = Child.get();
			}
		}
		return BestChild;
	}

	void RunIteration(const chess::Board& RootBoard, FMctsNode& RootNode, const chess::Color RootColor, const FChessMctsSearchSettings& Settings, std::mt19937_64& RandomGenerator)
	{
		chess::Board Board = RootBoard;
		FMctsNode* Node = &RootNode;
		while (Node->UnexpandedMoves.empty() && !Node->Children.empty())
		{
			Node = SelectChild(*Node, Board.sideToMove() == RootColor, Settings.ExplorationConstant);
			Board.makeMove(Node->MoveFromParent);
		}

		if (!Node->UnexpandedMoves.empty())
		{
			std::uniform_int_distribution<size_t> Distribution(0, Node->UnexpandedMoves.size() - 1);
			const size_t MoveIndex = Distribution(RandomGenerator);
			const chess::Move Move = Node->UnexpandedMoves[MoveIndex];
			Node->UnexpandedMoves[MoveIndex] = Node->UnexpandedMoves.back();
			Node->UnexpandedMoves.pop_back();
			Board.makeMove(Move);

			Node->Children.push_back(std::make_unique<FMctsNode>(Node, Move, Board));
			Node = Node->Children.back().get();
		}

		const double Value = EvaluatePosition(Board, RootColor, Settings);
		while (Node)
		{
			++Node->Visits;
			Node->ValueSum += Value;
			Node = Node->Parent;
		}
	}

	const FMctsNode& SelectFinalChild(const FMctsNode& RootNode)
	{
		return **std::max_element(RootNode.Children.begin(), RootNode.Children.end(), [](const std::unique_ptr<FMctsNode>& Left, const std::unique_ptr<FMctsNode>& Right)
		{
			if (Left->Visits != Right->Visits)
			{
				return Left->Visits < Right->Visits;
			}
			return Left->ValueSum / static_cast<double>(Left->Visits) < Right->ValueSum / static_cast<double>(Right->Visits);
		});
	}

	void LogSearchSummary(const chess::Board& Board, const FMctsNode& RootNode, const FMctsNode& SelectedChild, const FChessBotSearchResult& Result, const int32 LogCandidateCount)
	{
		const FString SelectedMove = UTF8_TO_TCHAR(chess::uci::moveToUci(SelectedChild.MoveFromParent, Board.chess960()).c_str());
		const size_t RootMoveCount = RootNode.Children.size() + RootNode.UnexpandedMoves.size();
		UE_LOG(LogChessBot, Log, TEXT("MCTS search complete | Iterations=%llu | RootMoves=%llu | ExpandedCandidates=%llu | Selected=%s | Elapsed=%.3fs"), static_cast<unsigned long long>(Result.SearchedNodes), static_cast<unsigned long long>(RootMoveCount), static_cast<unsigned long long>(RootNode.Children.size()), *SelectedMove, Result.ElapsedSeconds);

		std::vector<const FMctsNode*> RankedCandidates;
		RankedCandidates.reserve(RootNode.Children.size());
		for (const std::unique_ptr<FMctsNode>& Child : RootNode.Children)
		{
			RankedCandidates.push_back(Child.get());
		}
		std::sort(RankedCandidates.begin(), RankedCandidates.end(), [](const FMctsNode* Left, const FMctsNode* Right)
		{
			if (Left->Visits != Right->Visits)
			{
				return Left->Visits > Right->Visits;
			}
			return Left->ValueSum / static_cast<double>(Left->Visits) > Right->ValueSum / static_cast<double>(Right->Visits);
		});

		const int32 CandidateCount = std::min(LogCandidateCount, static_cast<int32>(RankedCandidates.size()));
		for (int32 CandidateIndex = 0; CandidateIndex < CandidateCount; ++CandidateIndex)
		{
			const FMctsNode& Candidate = *RankedCandidates[CandidateIndex];
			const FString UciMove = UTF8_TO_TCHAR(chess::uci::moveToUci(Candidate.MoveFromParent, Board.chess960()).c_str());
			const double VisitPercent = 100.0 * static_cast<double>(Candidate.Visits) / static_cast<double>(Result.SearchedNodes);
			const double MeanValue = Candidate.ValueSum / static_cast<double>(Candidate.Visits);
			UE_LOG(LogChessBot, Log, TEXT("MCTS candidate #%d | Move=%s | Visits=%llu | VisitRate=%.2f%% | MeanValue=%.4f"), CandidateIndex + 1, *UciMove, static_cast<unsigned long long>(Candidate.Visits), VisitPercent, MeanValue);
		}
	}
}

FChessBotSearchResult FChessMctsBot::FindMove(const FChessBotSearchRequest& Request, const FChessBotCancellationToken& CancellationToken)
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

	chess::Movelist RootMoves;
	chess::movegen::legalmoves(RootMoves, Board);
	if (RootMoves.empty())
	{
		Result.Status = EChessBotSearchStatus::NoLegalMoves;
		Result.ElapsedSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - StartTime).count();
		return Result;
	}

	std::mt19937_64 RandomGenerator(Request.RandomSeed);
	if (!Request.PreferredOpeningEcos.empty() && FChessOpeningTree::Get().TrySelectMove(Board, Request.PreferredOpeningEcos, RandomGenerator, Result.UciMove))
	{
		Result.Status = EChessBotSearchStatus::MoveFound;
		Result.ElapsedSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - StartTime).count();
		if (Request.MctsSettings.bLogSearch)
		{
			const FString OpeningMove = UTF8_TO_TCHAR(Result.UciMove.c_str());
			UE_LOG(LogChessBot, Log, TEXT("Opening book move selected | Iterations=0 | Move=%s | PreferredEcos=%llu | Elapsed=%.3fs"), *OpeningMove, static_cast<unsigned long long>(Request.PreferredOpeningEcos.size()), Result.ElapsedSeconds);
		}
		return Result;
	}

	FMctsNode RootNode(nullptr, chess::Move::NO_MOVE, Board);
	const chess::Color RootColor = Board.sideToMove();
	const bool bUseTimeLimit = Request.TimeLimitSeconds > 0.0;
	const bool bUseIterationLimit = Request.MctsSettings.IterationLimit > 0;
	const auto Deadline = StartTime + std::chrono::duration<double>(std::max(0.0, Request.TimeLimitSeconds));
	do
	{
		if (CancellationToken.IsCancellationRequested())
		{
			Result.Status = EChessBotSearchStatus::Cancelled;
			Result.ElapsedSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - StartTime).count();
			return Result;
		}

		RunIteration(Board, RootNode, RootColor, Request.MctsSettings, RandomGenerator);
		++Result.SearchedNodes;
	}
	while ((!bUseTimeLimit || std::chrono::steady_clock::now() < Deadline) && (!bUseIterationLimit || Result.SearchedNodes < Request.MctsSettings.IterationLimit) && (bUseTimeLimit || bUseIterationLimit));

	const FMctsNode& SelectedChild = SelectFinalChild(RootNode);
	Result.Status = EChessBotSearchStatus::MoveFound;
	Result.UciMove = chess::uci::moveToUci(SelectedChild.MoveFromParent, Board.chess960());
	Result.ElapsedSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - StartTime).count();
	if (Request.MctsSettings.bLogSearch)
	{
		LogSearchSummary(Board, RootNode, SelectedChild, Result, Request.MctsSettings.LogCandidateCount);
	}
	return Result;
}
