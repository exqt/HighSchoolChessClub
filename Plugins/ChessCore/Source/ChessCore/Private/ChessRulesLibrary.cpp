// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChessRulesLibrary.h"
#include "ChessGameState.h"

UChessGameState* UChessRulesLibrary::CreateChessGameState(UObject* WorldContextObject)
{
	UObject* Outer = WorldContextObject ? WorldContextObject : GetTransientPackage();
	return NewObject<UChessGameState>(Outer);
}
