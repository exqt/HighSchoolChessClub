// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ChessBotEngine.h"

class FChessRandomBot final : public IChessBotEngine
{
public:
	virtual FChessBotSearchResult FindMove(const FChessBotSearchRequest& Request, const FChessBotCancellationToken& CancellationToken) override;
};
