// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ChessBotEngine.h"

#include <vector>

namespace chess
{
	class Board;
	class Color;
	class Move;
}

namespace ChessMctsHeuristics
{
	double EvaluatePosition(const chess::Board& Board, chess::Color RootColor, const FChessMctsSearchSettings& Settings);
	std::vector<double> CalculateMovePriors(const chess::Board& Board, const std::vector<chess::Move>& Moves, const FChessMctsSearchSettings& Settings);
}
