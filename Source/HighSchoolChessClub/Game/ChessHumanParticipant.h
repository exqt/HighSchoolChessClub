#pragma once

#include "CoreMinimal.h"
#include "ChessCoreTypes.h"
#include "Game/ChessParticipant.h"
#include "ChessHumanParticipant.generated.h"

UCLASS(BlueprintType, Blueprintable)
class HIGHSCHOOLCHESSCLUB_API UChessHumanParticipant : public UChessParticipant
{
	GENERATED_BODY()

public:
	virtual void BeginTurn() override;
	virtual void EndTurn() override;

	void AttachInputSource(AActor* InInputSource);
	void DetachInputSource(AActor* InInputSource);

	bool MoveCursor(FIntPoint Delta);
	bool SelectCurrentSquare();
	void CancelSelection();

private:
	UPROPERTY(Transient)
	TArray<FChessCoreMove> SelectedLegalMoves;

	TWeakObjectPtr<AActor> InputSource;
	FIntPoint SelectedSquare = FIntPoint::ZeroValue;
	bool bHasSelectedSquare = false;

	bool CanAcceptInput() const;
	bool SelectPieceAtCursor();
	TArray<FIntPoint> GetLegalDestinationSquares() const;
	void RefreshCursorVisibility() const;
};
