// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ChessCoreTypes.generated.h"

UENUM(BlueprintType)
enum class EChessCorePieceColor : uint8
{
	White,
	Black,
	None
};

UENUM(BlueprintType)
enum class EChessCorePieceType : uint8
{
	None,
	Pawn,
	Knight,
	Bishop,
	Rook,
	Queen,
	King
};

UENUM(BlueprintType)
enum class EChessCoreGameResult : uint8
{
	None,
	WhiteWin,
	BlackWin,
	Draw
};

UENUM(BlueprintType)
enum class EChessCoreGameResultReason : uint8
{
	None,
	Checkmate,
	Stalemate,
	InsufficientMaterial,
	FiftyMoveRule,
	ThreefoldRepetition
};

USTRUCT(BlueprintType)
struct CHESSCORE_API FChessCoreSquare
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chess", meta=(ClampMin="0", ClampMax="7"))
	int32 File = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chess", meta=(ClampMin="0", ClampMax="7"))
	int32 Rank = 0;

	FChessCoreSquare() = default;
	FChessCoreSquare(int32 InFile, int32 InRank)
		: File(InFile)
		, Rank(InRank)
	{
	}

	bool IsValid() const
	{
		return File >= 0 && File < 8 && Rank >= 0 && Rank < 8;
	}
};

USTRUCT(BlueprintType)
struct CHESSCORE_API FChessCoreMove
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chess")
	FChessCoreSquare From;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chess")
	FChessCoreSquare To;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chess")
	EChessCorePieceType Promotion = EChessCorePieceType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chess")
	FString Uci;
};

USTRUCT(BlueprintType)
struct CHESSCORE_API FChessCorePiece
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chess")
	FChessCoreSquare Square;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chess")
	EChessCorePieceType Type = EChessCorePieceType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chess")
	EChessCorePieceColor Color = EChessCorePieceColor::None;
};

USTRUCT(BlueprintType)
struct CHESSCORE_API FChessCoreGameStatus
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chess")
	bool bInCheck = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chess")
	EChessCoreGameResult Result = EChessCoreGameResult::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chess")
	EChessCoreGameResultReason Reason = EChessCoreGameResultReason::None;
};
