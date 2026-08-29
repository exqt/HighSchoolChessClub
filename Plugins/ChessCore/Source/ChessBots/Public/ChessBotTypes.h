// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ChessBotTypes.generated.h"

UENUM(BlueprintType)
enum class EChessBotType : uint8
{
	Random,
	Mcts UMETA(DisplayName="MCTS")
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
struct CHESSBOTS_API FChessMctsSettings
{
	GENERATED_BODY()

	// A value of zero uses only TimeLimitSeconds.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ChessMctsSettings", meta=(ClampMin="0"))
	int32 IterationLimit = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ChessMctsSettings", meta=(ClampMin="0.0"))
	float ExplorationConstant = 1.41421356f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ChessMctsSettings", meta=(ClampMin="0.001"))
	float MaterialScoreScale = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ChessMctsSettings", meta=(ClampMin="0.0"))
	float PawnValue = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ChessMctsSettings", meta=(ClampMin="0.0"))
	float KnightValue = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ChessMctsSettings", meta=(ClampMin="0.0"))
	float BishopValue = 3.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ChessMctsSettings", meta=(ClampMin="0.0"))
	float RookValue = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ChessMctsSettings", meta=(ClampMin="0.0"))
	float QueenValue = 9.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ChessMctsSettings")
	bool bLogSearch = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ChessMctsSettings", meta=(ClampMin="0", EditCondition="bLogSearch"))
	int32 LogCandidateCount = 5;
};

USTRUCT(BlueprintType)
struct CHESSBOTS_API FChessBotSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ChessBotSettings")
	EChessBotType BotType = EChessBotType::Random;

	// A value of zero creates a seed from the request ID.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ChessBotSettings", meta=(ClampMin="0"))
	int32 RandomSeed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ChessBotSettings", meta=(ClampMin="0.0"))
	float TimeLimitSeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ChessBotSettings", meta=(DisplayName="Preferred Openings (ECO)", EditCondition="BotType == EChessBotType::Mcts", EditConditionHides))
	TArray<FString> PreferredOpenings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ChessBotSettings", meta=(DisplayName="MCTS Settings", EditCondition="BotType == EChessBotType::Mcts", EditConditionHides))
	FChessMctsSettings MctsSettings;
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
