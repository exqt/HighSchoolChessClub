#pragma once

#include "CoreMinimal.h"
#include "ChessBotTypes.h"
#include "ChessParticipant.h"
#include "ChessBotParticipant.generated.h"

UCLASS(BlueprintType, Blueprintable)
class HIGHSCHOOLCHESSCLUB_API UChessBotParticipant : public UChessParticipant
{
	GENERATED_BODY()

public:
	virtual void BeginTurn() override;
	virtual void EndTurn() override;
	void FinishChessMoveAnimation(int32 MoveId);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="ChessBotParticipant")
	FChessBotSettings BotSettings;

private:
	UFUNCTION()
	void OnMoveReady(const FChessBotResult& Result);

	FGuid PendingRequestId;
	FChessCoreMove PendingMove;
	int32 PendingMoveId = INDEX_NONE;
	int32 LastMoveId = 0;
};
