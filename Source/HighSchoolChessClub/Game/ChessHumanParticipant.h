#pragma once

#include "CoreMinimal.h"
#include "ChessCoreTypes.h"
#include "Game/ChessParticipant.h"
#include "ChessHumanParticipant.generated.h"

/* 사람의 입력과 체스판 선택 상태를 처리하는 Participant */
UCLASS(BlueprintType, Blueprintable)
class HIGHSCHOOLCHESSCLUB_API UChessHumanParticipant : public UChessParticipant
{
	GENERATED_BODY()

public:
#pragma region Turn
	virtual void BeginTurn() override;
	virtual void EndTurn() override;
#pragma endregion

#pragma region Input
	bool MoveCursor(FIntPoint Delta);
	bool SetCursorSquare(FIntPoint Square);
	bool SelectCurrentSquare();
	void CancelSelection();
	void SetUsingPointerInput(bool bInUsingPointerInput);
#pragma endregion

private:
#pragma region Selection
	bool SelectPieceAtCursor();
	TArray<FIntPoint> GetLegalDestinationSquares() const;

	UPROPERTY(Transient)
	TArray<FChessCoreMove> SelectedLegalMoves;

	FIntPoint SelectedSquare = FIntPoint::ZeroValue;
	bool bHasSelectedSquare = false;
#pragma endregion

#pragma region Input State
	bool CanAcceptInput() const;
	void RefreshCursorVisibility() const;

	bool bUsingPointerInput = false;
#pragma endregion
};
