// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreTypes.h"

#include <atomic>
#include <string>
#include <vector>

enum class EChessBotSearchStatus : uint8
{
	MoveFound,
	NoLegalMoves,
	InvalidPosition,
	Cancelled
};

struct CHESSBOTS_API FChessMctsSearchSettings
{
	uint64 IterationLimit = 0;
	double ExplorationConstant = 1.41421356237;
	double MaterialScoreScale = 10.0;
	double PawnValue = 1.0;
	double KnightValue = 3.0;
	double BishopValue = 3.25;
	double RookValue = 5.0;
	double QueenValue = 9.0;
	bool bLogSearch = true;
	int32 LogCandidateCount = 5;
};

struct CHESSBOTS_API FChessBotSearchRequest
{
	std::string Fen;
	std::vector<std::string> PreferredOpeningEcos;
	FChessMctsSearchSettings MctsSettings;
	uint64 RandomSeed = 0;
	double TimeLimitSeconds = 1.0;
};

struct CHESSBOTS_API FChessBotSearchResult
{
	EChessBotSearchStatus Status = EChessBotSearchStatus::InvalidPosition;
	std::string UciMove;
	uint64 SearchedNodes = 0;
	double ElapsedSeconds = 0.0;
};

class CHESSBOTS_API FChessBotCancellationToken
{
public:
	void Cancel() { bCancelled.store(true, std::memory_order_relaxed); }
	bool IsCancellationRequested() const { return bCancelled.load(std::memory_order_relaxed); }

private:
	std::atomic_bool bCancelled = false;
};

class CHESSBOTS_API IChessBotEngine
{
public:
	virtual ~IChessBotEngine() = default;

	virtual FChessBotSearchResult FindMove(const FChessBotSearchRequest& Request, const FChessBotCancellationToken& CancellationToken) = 0;
};
