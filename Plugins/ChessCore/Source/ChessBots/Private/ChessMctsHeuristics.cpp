// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChessMctsHeuristics.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>

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
	struct FTacticalHeatmap
	{
		std::array<double, 64> Attack{};
		std::array<double, 64> Defense{};

		double TotalAttack() const { return std::accumulate(Attack.begin(), Attack.end(), 0.0); }
		double TotalDefense() const { return std::accumulate(Defense.begin(), Defense.end(), 0.0); }
	};

	double GetPieceValue(const chess::PieceType PieceType, const FChessMctsSearchSettings& Settings)
	{
		switch (PieceType.internal())
		{
		case chess::PieceType::underlying::PAWN:
			return Settings.PawnValue;
		case chess::PieceType::underlying::KNIGHT:
			return Settings.KnightValue;
		case chess::PieceType::underlying::BISHOP:
			return Settings.BishopValue;
		case chess::PieceType::underlying::ROOK:
			return Settings.RookValue;
		case chess::PieceType::underlying::QUEEN:
			return Settings.QueenValue;
		case chess::PieceType::underlying::KING:
			return Settings.QueenValue + Settings.RookValue;
		default:
			return 0.0;
		}
	}

	chess::Bitboard GetAttacks(const chess::Board& Board, const chess::Color Color, const chess::PieceType PieceType, const chess::Square Square)
	{
		switch (PieceType.internal())
		{
		case chess::PieceType::underlying::PAWN:
			return chess::attacks::pawn(Color, Square);
		case chess::PieceType::underlying::KNIGHT:
			return chess::attacks::knight(Square);
		case chess::PieceType::underlying::BISHOP:
			return chess::attacks::bishop(Square, Board.occ());
		case chess::PieceType::underlying::ROOK:
			return chess::attacks::rook(Square, Board.occ());
		case chess::PieceType::underlying::QUEEN:
			return chess::attacks::queen(Square, Board.occ());
		case chess::PieceType::underlying::KING:
			return chess::attacks::king(Square);
		default:
			return {};
		}
	}

	double GetMaterialScore(const chess::Board& Board, const chess::Color Color, const FChessMctsSearchSettings& Settings)
	{
		const std::array<chess::PieceType, 5> PieceTypes = {chess::PieceType::PAWN, chess::PieceType::KNIGHT, chess::PieceType::BISHOP, chess::PieceType::ROOK, chess::PieceType::QUEEN};
		double Score = 0.0;
		for (const chess::PieceType PieceType : PieceTypes)
		{
			Score += GetPieceValue(PieceType, Settings) * Board.pieces(PieceType, Color).count();
		}
		return Score;
	}

	double GetCenterScore(const chess::Square Square)
	{
		return 7.0 - std::abs(static_cast<double>(Square.file()) - 3.5) - std::abs(static_cast<double>(Square.rank()) - 3.5);
	}

	int GetRelativeRank(const chess::Square Square, const chess::Color Color)
	{
		const int Rank = static_cast<int>(Square.rank());
		return Color == chess::Color::WHITE ? Rank : 7 - Rank;
	}

	int CountPawnsOnFile(const chess::Board& Board, const chess::Color Color, const int File)
	{
		return (Board.pieces(chess::PieceType::PAWN, Color) & chess::Bitboard(chess::File(File))).count();
	}

	double GetPieceActivityScore(const chess::Board& Board, const chess::Color Color)
	{
		double Score = 0.0;
		const std::array<chess::PieceType, 5> PieceTypes = {chess::PieceType::PAWN, chess::PieceType::KNIGHT, chess::PieceType::BISHOP, chess::PieceType::ROOK, chess::PieceType::QUEEN};
		for (const chess::PieceType PieceType : PieceTypes)
		{
			chess::Bitboard Pieces = Board.pieces(PieceType, Color);
			while (Pieces)
			{
				const chess::Square Square = Pieces.pop();
				const double CenterScore = GetCenterScore(Square);
				switch (PieceType.internal())
				{
				case chess::PieceType::underlying::PAWN:
					Score += std::max(0, GetRelativeRank(Square, Color) - 1) * 0.06 + CenterScore * 0.01;
					break;
				case chess::PieceType::underlying::KNIGHT:
					Score += CenterScore * 0.08;
					break;
				case chess::PieceType::underlying::BISHOP:
					Score += CenterScore * 0.04;
					break;
				case chess::PieceType::underlying::ROOK:
					Score += GetRelativeRank(Square, Color) == 6 ? 0.2 : 0.0;
					Score += CountPawnsOnFile(Board, Color, static_cast<int>(Square.file())) == 0 ? 0.08 : 0.0;
					Score += CountPawnsOnFile(Board, ~Color, static_cast<int>(Square.file())) == 0 ? 0.08 : 0.0;
					break;
				case chess::PieceType::underlying::QUEEN:
					Score += CenterScore * 0.015;
					break;
				default:
					break;
				}
			}
		}
		if (Board.pieces(chess::PieceType::BISHOP, Color).count() >= 2)
		{
			Score += 0.3;
		}
		return Score;
	}

	double GetMobilityScore(const chess::Board& Board, const chess::Color Color)
	{
		const std::array<chess::PieceType, 4> PieceTypes = {chess::PieceType::KNIGHT, chess::PieceType::BISHOP, chess::PieceType::ROOK, chess::PieceType::QUEEN};
		double Score = 0.0;
		for (const chess::PieceType PieceType : PieceTypes)
		{
			chess::Bitboard Pieces = Board.pieces(PieceType, Color);
			while (Pieces)
			{
				const chess::Square Square = Pieces.pop();
				Score += (GetAttacks(Board, Color, PieceType, Square) & ~Board.us(Color)).count() * 0.025;
			}
		}
		return Score;
	}

	bool IsPassedPawn(const chess::Board& Board, const chess::Color Color, const chess::Square PawnSquare)
	{
		chess::Bitboard EnemyPawns = Board.pieces(chess::PieceType::PAWN, ~Color);
		while (EnemyPawns)
		{
			const chess::Square EnemySquare = EnemyPawns.pop();
			if (std::abs(static_cast<int>(EnemySquare.file()) - static_cast<int>(PawnSquare.file())) <= 1 && GetRelativeRank(EnemySquare, Color) > GetRelativeRank(PawnSquare, Color))
			{
				return false;
			}
		}
		return true;
	}

	double GetPawnStructureScore(const chess::Board& Board, const chess::Color Color)
	{
		double Score = 0.0;
		for (int File = 0; File < 8; ++File)
		{
			const int PawnCount = CountPawnsOnFile(Board, Color, File);
			Score -= std::max(0, PawnCount - 1) * 0.15;
		}

		const chess::Bitboard OwnPawns = Board.pieces(chess::PieceType::PAWN, Color);
		chess::Bitboard Pawns = OwnPawns;
		while (Pawns)
		{
			const chess::Square Square = Pawns.pop();
			const int File = static_cast<int>(Square.file());
			const bool bHasAdjacentPawn = (File > 0 && CountPawnsOnFile(Board, Color, File - 1) > 0) || (File < 7 && CountPawnsOnFile(Board, Color, File + 1) > 0);
			Score -= bHasAdjacentPawn ? 0.0 : 0.12;
			Score += (chess::attacks::pawn(~Color, Square) & OwnPawns) ? 0.05 : 0.0;
			Score += IsPassedPawn(Board, Color, Square) ? 0.08 + std::max(0, GetRelativeRank(Square, Color) - 1) * 0.04 : 0.0;
		}
		return Score;
	}

	double GetKingSafetyScore(const chess::Board& Board, const chess::Color Color, const FChessMctsSearchSettings& Settings)
	{
		const chess::Square KingSquare = Board.kingSq(Color);
		double Score = 0.0;
		for (int RankOffset = 1; RankOffset <= 2; ++RankOffset)
		{
			const int ShieldRank = static_cast<int>(KingSquare.rank()) + (Color == chess::Color::WHITE ? RankOffset : -RankOffset);
			if (ShieldRank < 0 || ShieldRank > 7)
			{
				continue;
			}
			for (int FileOffset = -1; FileOffset <= 1; ++FileOffset)
			{
				const int ShieldFile = static_cast<int>(KingSquare.file()) + FileOffset;
				if (ShieldFile >= 0 && ShieldFile < 8 && Board.at(chess::Square(chess::File(ShieldFile), chess::Rank(ShieldRank))) == chess::Piece(chess::PieceType::PAWN, Color))
				{
					Score += RankOffset == 1 ? 0.12 : 0.05;
				}
			}
		}

		chess::Bitboard KingZone = chess::attacks::king(KingSquare) | chess::Bitboard::fromSquare(KingSquare);
		while (KingZone)
		{
			Score -= chess::attacks::attackers(Board, ~Color, KingZone.pop()).count() * 0.04;
		}

		const int KingFile = static_cast<int>(KingSquare.file());
		Score -= CountPawnsOnFile(Board, Color, KingFile) == 0 ? 0.1 : 0.0;
		Score += GetRelativeRank(KingSquare, Color) == 0 && (KingFile == 2 || KingFile == 6) ? 0.25 : 0.0;
		const double InitialNonPawnMaterial = 2.0 * Settings.KnightValue + 2.0 * Settings.BishopValue + 2.0 * Settings.RookValue + Settings.QueenValue;
		const double Phase = InitialNonPawnMaterial > 0.0 ? std::clamp((GetMaterialScore(Board, Color, Settings) - Settings.PawnValue * Board.pieces(chess::PieceType::PAWN, Color).count()) / InitialNonPawnMaterial, 0.0, 1.0) : 0.0;
		Score -= GetCenterScore(KingSquare) * 0.04 * Phase;
		return Score;
	}

	double GetThreatScore(const chess::Board& Board, const chess::Color Color, const FChessMctsSearchSettings& Settings)
	{
		double Score = 0.0;
		chess::Bitboard Targets = Board.them(Color) & ~Board.pieces(chess::PieceType::KING);
		while (Targets)
		{
			const chess::Square Square = Targets.pop();
			const int AttackerCount = chess::attacks::attackers(Board, Color, Square).count();
			if (AttackerCount == 0)
			{
				continue;
			}
			const int DefenderCount = chess::attacks::attackers(Board, ~Color, Square).count();
			const double Pressure = DefenderCount == 0 ? 0.12 : AttackerCount > DefenderCount ? 0.08 : 0.04;
			Score += GetPieceValue(Board.at(Square).type(), Settings) * Pressure;
		}
		return Score;
	}

	double GetWeightedPressure(const chess::Board& Board, chess::Bitboard Attackers, const double TargetValue, const FChessMctsSearchSettings& Settings)
	{
		const double MinimumAttackerValue = std::max(0.25, Settings.PawnValue);
		double Pressure = 0.0;
		while (Attackers)
		{
			Pressure += TargetValue / std::max(MinimumAttackerValue, GetPieceValue(Board.at(Attackers.pop()).type(), Settings));
		}
		return Pressure;
	}

	FTacticalHeatmap BuildTacticalHeatmap(const chess::Board& Board, const chess::Color Color, const FChessMctsSearchSettings& Settings)
	{
		FTacticalHeatmap Heatmap;
		chess::Bitboard OccupiedSquares = Board.occ();
		while (OccupiedSquares)
		{
			const chess::Square Square = OccupiedSquares.pop();
			const chess::Piece Piece = Board.at(Square);
			if (Piece.type() == chess::PieceType::KING)
			{
				continue;
			}
			const chess::Bitboard Attackers = chess::attacks::attackers(Board, Color, Square);
			const double Pressure = GetWeightedPressure(Board, Attackers, GetPieceValue(Piece.type(), Settings), Settings);
			if (Piece.color() == Color)
			{
				Heatmap.Defense[Square.index()] = Pressure;
			}
			else
			{
				Heatmap.Attack[Square.index()] = Pressure;
			}
		}
		return Heatmap;
	}
}

double ChessMctsHeuristics::EvaluatePosition(const chess::Board& Board, const chess::Color RootColor, const FChessMctsSearchSettings& Settings)
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

	const chess::Color OpponentColor = ~RootColor;
	const double Material = GetMaterialScore(Board, RootColor, Settings) - GetMaterialScore(Board, OpponentColor, Settings);
	const double PieceActivity = GetPieceActivityScore(Board, RootColor) - GetPieceActivityScore(Board, OpponentColor);
	const double Mobility = GetMobilityScore(Board, RootColor) - GetMobilityScore(Board, OpponentColor);
	const double PawnStructure = GetPawnStructureScore(Board, RootColor) - GetPawnStructureScore(Board, OpponentColor);
	const double KingSafety = GetKingSafetyScore(Board, RootColor, Settings) - GetKingSafetyScore(Board, OpponentColor, Settings);
	const double Threat = GetThreatScore(Board, RootColor, Settings) - GetThreatScore(Board, OpponentColor, Settings);
	const double Score = Settings.MaterialWeight * Material + Settings.PieceActivityWeight * PieceActivity + Settings.MobilityWeight * Mobility + Settings.PawnStructureWeight * PawnStructure + Settings.KingSafetyWeight * KingSafety + Settings.ThreatWeight * Threat;
	return std::tanh(Score / Settings.MaterialScoreScale);
}

std::vector<double> ChessMctsHeuristics::CalculateMovePriors(const chess::Board& Board, const std::vector<chess::Move>& Moves, const FChessMctsSearchSettings& Settings)
{
	std::vector<double> Priors;
	Priors.reserve(Moves.size());
	const chess::Color MovingColor = Board.sideToMove();
	const FTacticalHeatmap BaseHeatmap = BuildTacticalHeatmap(Board, MovingColor, Settings);
	const double PawnScale = std::max(0.25, Settings.PawnValue);
	for (const chess::Move Move : Moves)
	{
		FTacticalHeatmap BeforeHeatmap = BaseHeatmap;
		const chess::PieceType CapturedPieceType = Board.getCapturing<chess::PieceType>(Move);
		if (CapturedPieceType != chess::PieceType::NONE)
		{
			const chess::Square CapturedSquare = Move.typeOf() == chess::Move::ENPASSANT ? Move.to().ep_square() : Move.to();
			BeforeHeatmap.Attack[CapturedSquare.index()] = 0.0;
		}

		chess::Board NextBoard = Board;
		NextBoard.makeMove(Move);
		const FTacticalHeatmap NextHeatmap = BuildTacticalHeatmap(NextBoard, MovingColor, Settings);
		const double AttackDelta = std::clamp(NextHeatmap.TotalAttack() - BeforeHeatmap.TotalAttack(), -2.0 * Settings.QueenValue, 2.0 * Settings.QueenValue);
		const double DefenseDelta = std::clamp(NextHeatmap.TotalDefense() - BeforeHeatmap.TotalDefense(), -2.0 * Settings.QueenValue, 2.0 * Settings.QueenValue);
		double Prior = 1.0;
		Prior += Board.givesCheck(Move) != chess::CheckType::NO_CHECK ? Settings.CheckPolicyWeight : 0.0;
		Prior += Settings.CapturePolicyWeight * GetPieceValue(CapturedPieceType, Settings) / PawnScale;
		Prior += Settings.AttackPolicyWeight * AttackDelta;
		Prior += Settings.DefensePolicyWeight * DefenseDelta;
		if (Move.typeOf() == chess::Move::PROMOTION)
		{
			Prior += Settings.CapturePolicyWeight * (GetPieceValue(Move.promotionType(), Settings) - Settings.PawnValue) / PawnScale;
		}
		Priors.push_back(std::max(0.05, Prior));
	}

	const double PriorSum = std::accumulate(Priors.begin(), Priors.end(), 0.0);
	for (double& Prior : Priors)
	{
		Prior /= PriorSum;
	}
	return Priors;
}
