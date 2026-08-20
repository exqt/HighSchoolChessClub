// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ChessBotTypes.generated.h"

UENUM(BlueprintType)
enum class EChessBotType : uint8
{
	Random
};

UENUM(BlueprintType)
enum class EChessBotResultStatus : uint8
{
	MoveFound,
	NoLegalMoves,
	InvalidPosition,
	Cancelled
};

USTRUCT(BlueprintType)
struct CHESSBOTS_API FChessBotSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chess Bot")
	EChessBotType BotType = EChessBotType::Random;

	// A value of zero creates a seed from the request ID.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chess Bot", meta=(ClampMin="0"))
	int32 RandomSeed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chess Bot", meta=(ClampMin="0.0"))
	float TimeLimitSeconds = 1.0f;
};

USTRUCT(BlueprintType)
struct CHESSBOTS_API FChessBotResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Chess Bot")
	FGuid RequestId;

	UPROPERTY(BlueprintReadOnly, Category="Chess Bot")
	EChessBotResultStatus Status = EChessBotResultStatus::InvalidPosition;

	UPROPERTY(BlueprintReadOnly, Category="Chess Bot")
	FString UciMove;

	UPROPERTY(BlueprintReadOnly, Category="Chess Bot")
	int64 SearchedNodes = 0;

	UPROPERTY(BlueprintReadOnly, Category="Chess Bot")
	float ElapsedSeconds = 0.0f;

	bool HasMove() const { return Status == EChessBotResultStatus::MoveFound && !UciMove.IsEmpty(); }
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnChessBotMoveReady, const FChessBotResult&, Result);
