// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include <cstddef>
#include <cstdint>
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace chess
{
	class Board;
}

class FChessOpeningTree final
{
public:
	static const FChessOpeningTree& Get();
	bool TrySelectMove(const chess::Board& Board, const std::vector<std::string>& PreferredEcos, std::mt19937_64& RandomGenerator, std::string& OutUciMove) const;

private:
	struct FNode
	{
		std::unordered_map<std::string, size_t> Children;
		std::unordered_set<std::string> Ecos;
	};

	FChessOpeningTree();
	void InsertLine(const std::string& Eco, const std::vector<std::string>& Moves);

	std::vector<FNode> Nodes;
	std::unordered_map<uint64_t, std::vector<size_t>> NodesByPosition;
};
