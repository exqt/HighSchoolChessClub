// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ChessRulesLibrary.generated.h"

class UChessGameState;

UCLASS()
class CHESSCORE_API UChessRulesLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Chess", meta=(WorldContext="WorldContextObject"))
	static UChessGameState* CreateChessGameState(UObject* WorldContextObject);
};
