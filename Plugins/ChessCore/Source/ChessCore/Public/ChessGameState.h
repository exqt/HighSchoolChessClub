// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ChessCoreTypes.h"
#include "UObject/Object.h"
#include "ChessGameState.generated.h"

class FChessGameStateImpl;

UCLASS(BlueprintType)
class CHESSCORE_API UChessGameState : public UObject
{
	GENERATED_BODY()

public:
	UChessGameState();
	virtual ~UChessGameState() override;
	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintCallable, Category="Chess")
	void ResetToStartPosition();

	UFUNCTION(BlueprintCallable, Category="Chess")
	bool SetFen(const FString& Fen);

	UFUNCTION(BlueprintPure, Category="Chess")
	FString GetFen(bool bIncludeMoveCounters = true) const;

	UFUNCTION(BlueprintPure, Category="Chess")
	EChessCorePieceColor GetSideToMove() const;

	UFUNCTION(BlueprintPure, Category="Chess")
	bool IsInCheck() const;

	UFUNCTION(BlueprintCallable, Category="Chess")
	bool GetLegalMoves(TArray<FChessCoreMove>& OutMoves) const;

	UFUNCTION(BlueprintCallable, Category="Chess")
	bool TryMakeMove(const FChessCoreMove& Move, FChessCoreMove& AppliedMove);

	UFUNCTION(BlueprintCallable, Category="Chess")
	bool TryMakeMoveUci(const FString& UciMove, FChessCoreMove& AppliedMove);

	UFUNCTION(BlueprintCallable, Category="Chess")
	bool GetPieces(TArray<FChessCorePiece>& OutPieces) const;

	UFUNCTION(BlueprintCallable, Category="ChessGameState")
	void GetCapturedPieces(TArray<FChessCorePiece>& OutCapturedPieces) const;

	UFUNCTION(BlueprintCallable, Category="ChessGameState")
	void GetMoveHistory(TArray<FChessCoreMove>& OutMoveHistory) const;

	UFUNCTION(BlueprintPure, Category="Chess")
	FChessCoreGameStatus GetGameStatus() const;

private:
	UPROPERTY(Transient)
	TArray<FChessCorePiece> CapturedPieces;

	UPROPERTY(Transient)
	TArray<FChessCoreMove> MoveHistory;

	FChessGameStateImpl* Impl = nullptr;
};
