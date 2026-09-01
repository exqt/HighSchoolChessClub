// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChessMctsBot.h"

#include "ChessBotLog.h"
#include "ChessMctsHeuristics.h"
#include "ChessOpeningTree.h"

#include <algorithm>
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
	enum class EMctsProvenResult
	{
		Unknown,
		Win,
		Draw,
		Loss
	};

	struct FMctsNode
	{
		FMctsNode(FMctsNode* InParent, const chess::Move InMove, const chess::Board& Board, const double InPrior)
			: Parent(InParent)
			, MoveFromParent(InMove)
			, Prior(InPrior)
		{
			const chess::GameResult GameResult = Board.isGameOver().second;
			if (GameResult != chess::GameResult::NONE)
			{
				ProvenResult = GameResult == chess::GameResult::WIN ? EMctsProvenResult::Win : GameResult == chess::GameResult::DRAW ? EMctsProvenResult::Draw : EMctsProvenResult::Loss;
				MateDistance = GameResult == chess::GameResult::DRAW ? -1 : 0;
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
		std::vector<double> UnexpandedPriors;
		uint64 Visits = 0;
		double ValueSum = 0.0;
		double Prior = 1.0;
		EMctsProvenResult ProvenResult = EMctsProvenResult::Unknown;
		int32 MateDistance = -1;
		bool bHasPolicyPriors = false;
		bool bImmediateWinsScanned = false;
	};

	const TCHAR* GetProvenResultName(const EMctsProvenResult Result)
	{
		switch (Result)
		{
		case EMctsProvenResult::Win:
			return TEXT("Win");
		case EMctsProvenResult::Draw:
			return TEXT("Draw");
		case EMctsProvenResult::Loss:
			return TEXT("Loss");
		default:
			return TEXT("Unknown");
		}
	}

	void UpdateProvenResult(FMctsNode& Node)
	{
		if (Node.ProvenResult != EMctsProvenResult::Unknown)
		{
			return;
		}

		int32 ShortestWinDistance = std::numeric_limits<int32>::max();
		for (const std::unique_ptr<FMctsNode>& Child : Node.Children)
		{
			if (Child->ProvenResult == EMctsProvenResult::Loss)
			{
				ShortestWinDistance = std::min(ShortestWinDistance, Child->MateDistance + 1);
			}
		}
		if (ShortestWinDistance != std::numeric_limits<int32>::max())
		{
			Node.ProvenResult = EMctsProvenResult::Win;
			Node.MateDistance = ShortestWinDistance;
			return;
		}

		if (!Node.UnexpandedMoves.empty() || Node.Children.empty())
		{
			return;
		}

		bool bHasDraw = false;
		int32 LongestLossDistance = 0;
		for (const std::unique_ptr<FMctsNode>& Child : Node.Children)
		{
			if (Child->ProvenResult == EMctsProvenResult::Unknown)
			{
				return;
			}
			bHasDraw |= Child->ProvenResult == EMctsProvenResult::Draw;
			if (Child->ProvenResult == EMctsProvenResult::Win)
			{
				LongestLossDistance = std::max(LongestLossDistance, Child->MateDistance + 1);
			}
		}

		Node.ProvenResult = bHasDraw ? EMctsProvenResult::Draw : EMctsProvenResult::Loss;
		Node.MateDistance = bHasDraw ? -1 : LongestLossDistance;
	}

	EMctsProvenResult GetMoveResult(const EMctsProvenResult ChildResult)
	{
		if (ChildResult == EMctsProvenResult::Win)
		{
			return EMctsProvenResult::Loss;
		}
		if (ChildResult == EMctsProvenResult::Loss)
		{
			return EMctsProvenResult::Win;
		}
		return ChildResult;
	}

	double GetProvenValue(const EMctsProvenResult Result, const chess::Color SideToMove, const chess::Color RootColor)
	{
		if (Result == EMctsProvenResult::Draw)
		{
			return 0.0;
		}
		const bool bRootSideWins = (Result == EMctsProvenResult::Win) == (SideToMove == RootColor);
		return bRootSideWins ? 1.0 : -1.0;
	}

	bool AreTreeSettingsCompatible(const FChessMctsSearchSettings& Left, const FChessMctsSearchSettings& Right)
	{
		return Left.MaterialScoreScale == Right.MaterialScoreScale && Left.PawnValue == Right.PawnValue && Left.KnightValue == Right.KnightValue && Left.BishopValue == Right.BishopValue && Left.RookValue == Right.RookValue && Left.QueenValue == Right.QueenValue && Left.MaterialWeight == Right.MaterialWeight && Left.PieceActivityWeight == Right.PieceActivityWeight && Left.MobilityWeight == Right.MobilityWeight && Left.PawnStructureWeight == Right.PawnStructureWeight && Left.KingSafetyWeight == Right.KingSafetyWeight && Left.ThreatWeight == Right.ThreatWeight && Left.CheckPolicyWeight == Right.CheckPolicyWeight && Left.CapturePolicyWeight == Right.CapturePolicyWeight && Left.AttackPolicyWeight == Right.AttackPolicyWeight && Left.DefensePolicyWeight == Right.DefensePolicyWeight;
	}

	bool FindSubtreePath(const FMctsNode& Node, chess::Board& Board, const std::string& TargetFen, const int32 RemainingDepth, std::vector<size_t>& OutPath)
	{
		if (RemainingDepth == 0)
		{
			return false;
		}
		for (size_t ChildIndex = 0; ChildIndex < Node.Children.size(); ++ChildIndex)
		{
			const FMctsNode& Child = *Node.Children[ChildIndex];
			Board.makeMove(Child.MoveFromParent);
			OutPath.push_back(ChildIndex);
			if (Board.getFen() == TargetFen || FindSubtreePath(Child, Board, TargetFen, RemainingDepth - 1, OutPath))
			{
				return true;
			}
			OutPath.pop_back();
			Board.unmakeMove(Child.MoveFromParent);
		}
		return false;
	}

	FMctsNode* SelectChild(FMctsNode& Node, const bool bRootTurn, const double ExplorationConstant, const double PolicyPriorStrength)
	{
		FMctsNode* BestChild = nullptr;
		double BestScore = -std::numeric_limits<double>::infinity();
		const double LogParentVisits = std::log(static_cast<double>(std::max<uint64>(1, Node.Visits)));
		const bool bHasUnprovenChild = std::any_of(Node.Children.begin(), Node.Children.end(), [](const std::unique_ptr<FMctsNode>& Child) { return Child->ProvenResult == EMctsProvenResult::Unknown; });
		for (const std::unique_ptr<FMctsNode>& Child : Node.Children)
		{
			if (bHasUnprovenChild && Child->ProvenResult != EMctsProvenResult::Unknown)
			{
				continue;
			}
			const double MeanValue = Child->ValueSum / static_cast<double>(Child->Visits);
			const double Exploration = ExplorationConstant * std::sqrt(LogParentVisits / static_cast<double>(Child->Visits));
			const double PolicyBias = PolicyPriorStrength * Child->Prior * std::sqrt(static_cast<double>(std::max<uint64>(1, Node.Visits))) / (1.0 + static_cast<double>(Child->Visits));
			const double Score = (bRootTurn ? MeanValue : -MeanValue) + Exploration + PolicyBias;
			if (Score > BestScore)
			{
				BestScore = Score;
				BestChild = Child.get();
			}
		}
		return BestChild;
	}

	size_t SelectExpansionMove(const chess::Board& Board, FMctsNode& Node, std::mt19937_64& RandomGenerator)
	{
		if (!Node.bImmediateWinsScanned)
		{
			Node.bImmediateWinsScanned = true;
			for (size_t MoveIndex = 0; MoveIndex < Node.UnexpandedMoves.size(); ++MoveIndex)
			{
				if (Board.givesCheck(Node.UnexpandedMoves[MoveIndex]) == chess::CheckType::NO_CHECK)
				{
					continue;
				}
				chess::Board NextBoard = Board;
				NextBoard.makeMove(Node.UnexpandedMoves[MoveIndex]);
				if (NextBoard.isGameOver().second == chess::GameResult::LOSE)
				{
					return MoveIndex;
				}
			}
		}

		std::discrete_distribution<size_t> Distribution(Node.UnexpandedPriors.begin(), Node.UnexpandedPriors.end());
		return Distribution(RandomGenerator);
	}

	void RunIteration(const chess::Board& RootBoard, FMctsNode& RootNode, const chess::Color RootColor, const FChessMctsSearchSettings& Settings, std::mt19937_64& RandomGenerator)
	{
		chess::Board Board = RootBoard;
		FMctsNode* Node = &RootNode;
		while (Node->ProvenResult == EMctsProvenResult::Unknown && Node->UnexpandedMoves.empty() && !Node->Children.empty())
		{
			Node = SelectChild(*Node, Board.sideToMove() == RootColor, Settings.ExplorationConstant, Settings.PolicyPriorStrength);
			Board.makeMove(Node->MoveFromParent);
		}

		if (Node->ProvenResult == EMctsProvenResult::Unknown && !Node->UnexpandedMoves.empty())
		{
			if (!Node->bHasPolicyPriors)
			{
				Node->UnexpandedPriors = ChessMctsHeuristics::CalculateMovePriors(Board, Node->UnexpandedMoves, Settings);
				Node->bHasPolicyPriors = true;
			}
			const size_t MoveIndex = SelectExpansionMove(Board, *Node, RandomGenerator);
			const chess::Move Move = Node->UnexpandedMoves[MoveIndex];
			const double MovePrior = Node->UnexpandedPriors[MoveIndex];
			Node->UnexpandedMoves[MoveIndex] = Node->UnexpandedMoves.back();
			Node->UnexpandedMoves.pop_back();
			Node->UnexpandedPriors[MoveIndex] = Node->UnexpandedPriors.back();
			Node->UnexpandedPriors.pop_back();
			Board.makeMove(Move);

			Node->Children.push_back(std::make_unique<FMctsNode>(Node, Move, Board, MovePrior));
			Node = Node->Children.back().get();
		}

		const double Value = Node->ProvenResult == EMctsProvenResult::Unknown ? ChessMctsHeuristics::EvaluatePosition(Board, RootColor, Settings) : GetProvenValue(Node->ProvenResult, Board.sideToMove(), RootColor);
		while (Node)
		{
			++Node->Visits;
			Node->ValueSum += Value;
			UpdateProvenResult(*Node);
			Node = Node->Parent;
		}
	}

	int32 GetMoveResultRank(const EMctsProvenResult ChildResult)
	{
		switch (GetMoveResult(ChildResult))
		{
		case EMctsProvenResult::Win:
			return 2;
		case EMctsProvenResult::Loss:
			return 0;
		default:
			return 1;
		}
	}

	bool IsPreferredFinalChild(const FMctsNode& Left, const FMctsNode& Right)
	{
		const int32 LeftResultRank = GetMoveResultRank(Left.ProvenResult);
		const int32 RightResultRank = GetMoveResultRank(Right.ProvenResult);
		if (LeftResultRank != RightResultRank)
		{
			return LeftResultRank > RightResultRank;
		}
		if (Left.ProvenResult == EMctsProvenResult::Loss && Left.MateDistance != Right.MateDistance)
		{
			return Left.MateDistance < Right.MateDistance;
		}
		if (Left.ProvenResult == EMctsProvenResult::Win && Left.MateDistance != Right.MateDistance)
		{
			return Left.MateDistance > Right.MateDistance;
		}
		if (Left.Visits != Right.Visits)
		{
			return Left.Visits > Right.Visits;
		}
		return Left.ValueSum / static_cast<double>(Left.Visits) > Right.ValueSum / static_cast<double>(Right.Visits);
	}

	const FMctsNode& SelectFinalChild(const FMctsNode& RootNode)
	{
		return **std::max_element(RootNode.Children.begin(), RootNode.Children.end(), [](const std::unique_ptr<FMctsNode>& Left, const std::unique_ptr<FMctsNode>& Right)
		{
			return IsPreferredFinalChild(*Right, *Left);
		});
	}

	void LogSearchSummary(const chess::Board& Board, const FMctsNode& RootNode, const FMctsNode& SelectedChild, const FChessBotSearchResult& Result, const uint64 PreviousRootVisits, const int32 LogCandidateCount)
	{
		const FString SelectedMove = UTF8_TO_TCHAR(chess::uci::moveToUci(SelectedChild.MoveFromParent, Board.chess960()).c_str());
		const size_t RootMoveCount = RootNode.Children.size() + RootNode.UnexpandedMoves.size();
		UE_LOG(LogChessBot, Log, TEXT("MCTS search complete | NewIterations=%llu | PreviousVisits=%llu | TotalVisits=%llu | RootMoves=%llu | ExpandedCandidates=%llu | Proven=%s | MatePly=%d | Selected=%s | Elapsed=%.3fs"), static_cast<unsigned long long>(Result.SearchedNodes), static_cast<unsigned long long>(PreviousRootVisits), static_cast<unsigned long long>(RootNode.Visits), static_cast<unsigned long long>(RootMoveCount), static_cast<unsigned long long>(RootNode.Children.size()), GetProvenResultName(RootNode.ProvenResult), RootNode.MateDistance, *SelectedMove, Result.ElapsedSeconds);

		std::vector<const FMctsNode*> RankedCandidates;
		RankedCandidates.reserve(RootNode.Children.size());
		for (const std::unique_ptr<FMctsNode>& Child : RootNode.Children)
		{
			RankedCandidates.push_back(Child.get());
		}
		std::sort(RankedCandidates.begin(), RankedCandidates.end(), [](const FMctsNode* Left, const FMctsNode* Right)
		{
			return IsPreferredFinalChild(*Left, *Right);
		});

		const int32 CandidateCount = std::min(LogCandidateCount, static_cast<int32>(RankedCandidates.size()));
		for (int32 CandidateIndex = 0; CandidateIndex < CandidateCount; ++CandidateIndex)
		{
			const FMctsNode& Candidate = *RankedCandidates[CandidateIndex];
			const FString UciMove = UTF8_TO_TCHAR(chess::uci::moveToUci(Candidate.MoveFromParent, Board.chess960()).c_str());
			const double VisitPercent = 100.0 * static_cast<double>(Candidate.Visits) / static_cast<double>(RootNode.Visits);
			const double MeanValue = Candidate.ValueSum / static_cast<double>(Candidate.Visits);
			UE_LOG(LogChessBot, Log, TEXT("MCTS candidate #%d | Move=%s | Visits=%llu | VisitRate=%.2f%% | MeanValue=%.4f | Prior=%.4f | Proven=%s | MatePly=%d"), CandidateIndex + 1, *UciMove, static_cast<unsigned long long>(Candidate.Visits), VisitPercent, MeanValue, Candidate.Prior, GetProvenResultName(GetMoveResult(Candidate.ProvenResult)), Candidate.MateDistance < 0 ? -1 : Candidate.MateDistance + 1);
		}
	}
}

struct FChessMctsBot::FSearchTree
{
	void Clear()
	{
		Root.reset();
		bInitialized = false;
	}

	bool Prepare(const chess::Board& Board, const FChessMctsSearchSettings& InSettings, int32& OutReusedDepth)
	{
		OutReusedDepth = 0;
		if (bInitialized && RootColor == Board.sideToMove() && AreTreeSettingsCompatible(Settings, InSettings))
		{
			const std::string TargetFen = Board.getFen();
			if (RootBoard.getFen() == TargetFen)
			{
				return true;
			}

			chess::Board TraversalBoard = RootBoard;
			std::vector<size_t> Path;
			if (FindSubtreePath(*Root, TraversalBoard, TargetFen, 2, Path))
			{
				std::unique_ptr<FMctsNode>* Subtree = &Root;
				for (const size_t ChildIndex : Path)
				{
					Subtree = &(*Subtree)->Children[ChildIndex];
				}
				std::unique_ptr<FMctsNode> ReusedRoot = std::move(*Subtree);
				ReusedRoot->Parent = nullptr;
				Root = std::move(ReusedRoot);
				RootBoard = Board;
				Settings = InSettings;
				OutReusedDepth = static_cast<int32>(Path.size());
				return true;
			}
		}

		Root = std::make_unique<FMctsNode>(nullptr, chess::Move::NO_MOVE, Board, 1.0);
		RootBoard = Board;
		RootColor = Board.sideToMove();
		Settings = InSettings;
		bInitialized = true;
		return false;
	}

	std::unique_ptr<FMctsNode> Root;
	chess::Board RootBoard;
	chess::Color RootColor = chess::Color::NONE;
	FChessMctsSearchSettings Settings;
	bool bInitialized = false;
};

FChessMctsBot::FChessMctsBot()
	: SearchTree(std::make_unique<FSearchTree>())
{
}

FChessMctsBot::~FChessMctsBot() = default;

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
		SearchTree->Clear();
		Result.Status = EChessBotSearchStatus::InvalidPosition;
		return Result;
	}

	chess::Movelist RootMoves;
	chess::movegen::legalmoves(RootMoves, Board);
	if (RootMoves.empty())
	{
		SearchTree->Clear();
		Result.Status = EChessBotSearchStatus::NoLegalMoves;
		Result.ElapsedSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - StartTime).count();
		return Result;
	}

	std::mt19937_64 RandomGenerator(Request.RandomSeed);
	if (!Request.PreferredOpeningEcos.empty() && FChessOpeningTree::Get().TrySelectMove(Board, Request.PreferredOpeningEcos, RandomGenerator, Result.UciMove))
	{
		SearchTree->Clear();
		Result.Status = EChessBotSearchStatus::MoveFound;
		Result.ElapsedSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - StartTime).count();
		if (Request.MctsSettings.bLogSearch)
		{
			const FString OpeningMove = UTF8_TO_TCHAR(Result.UciMove.c_str());
			UE_LOG(LogChessBot, Log, TEXT("Opening book move selected | Iterations=0 | Move=%s | PreferredEcos=%llu | Elapsed=%.3fs"), *OpeningMove, static_cast<unsigned long long>(Request.PreferredOpeningEcos.size()), Result.ElapsedSeconds);
		}
		return Result;
	}

	int32 ReusedDepth = 0;
	const bool bReusedSubtree = SearchTree->Prepare(Board, Request.MctsSettings, ReusedDepth);
	FMctsNode& RootNode = *SearchTree->Root;
	const chess::Color RootColor = SearchTree->RootColor;
	const uint64 PreviousRootVisits = RootNode.Visits;
	if (bReusedSubtree && Request.MctsSettings.bLogSearch)
	{
		UE_LOG(LogChessBot, Log, TEXT("MCTS subtree reused | Depth=%d | PreservedVisits=%llu"), ReusedDepth, static_cast<unsigned long long>(PreviousRootVisits));
	}
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
	while (RootNode.ProvenResult == EMctsProvenResult::Unknown && (!bUseTimeLimit || std::chrono::steady_clock::now() < Deadline) && (!bUseIterationLimit || Result.SearchedNodes < Request.MctsSettings.IterationLimit) && (bUseTimeLimit || bUseIterationLimit));

	const FMctsNode& SelectedChild = SelectFinalChild(RootNode);
	Result.Status = EChessBotSearchStatus::MoveFound;
	Result.UciMove = chess::uci::moveToUci(SelectedChild.MoveFromParent, Board.chess960());
	Result.ElapsedSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - StartTime).count();
	if (Request.MctsSettings.bLogSearch)
	{
		LogSearchSummary(Board, RootNode, SelectedChild, Result, PreviousRootVisits, Request.MctsSettings.LogCandidateCount);
	}
	return Result;
}
