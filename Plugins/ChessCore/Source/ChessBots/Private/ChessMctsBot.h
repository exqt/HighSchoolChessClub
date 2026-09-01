// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ChessBotEngine.h"

#include <memory>

class FChessMctsBot final : public IChessBotEngine
{
public:
	FChessMctsBot();
	virtual ~FChessMctsBot() override;
	virtual FChessBotSearchResult FindMove(const FChessBotSearchRequest& Request, const FChessBotCancellationToken& CancellationToken) override;

private:
	struct FSearchTree;
	std::unique_ptr<FSearchTree> SearchTree;
};
