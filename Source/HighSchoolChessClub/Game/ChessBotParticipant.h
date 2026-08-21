#pragma once

#include "CoreMinimal.h"
#include "ChessBotTypes.h"
#include "Game/ChessParticipant.h"
#include "ChessBotParticipant.generated.h"

UCLASS(BlueprintType, Blueprintable)
class HIGHSCHOOLCHESSCLUB_API UChessBotParticipant : public UChessParticipant
{
	GENERATED_BODY()

public:
	virtual void BeginTurn() override;
	virtual void EndTurn() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Chess Bot")
	FChessBotSettings BotSettings;

private:
	UFUNCTION()
	void OnMoveReady(const FChessBotResult& Result);

	FGuid PendingRequestId;
};
