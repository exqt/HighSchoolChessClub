#pragma once

#include "CoreMinimal.h"
#include "Game/ChessParticipantTypes.h"
#include "UObject/Object.h"
#include "ChessParticipant.generated.h"

class AChessMatch;

UCLASS(Abstract, BlueprintType, Blueprintable)
class HIGHSCHOOLCHESSCLUB_API UChessParticipant : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(AChessMatch* InMatch, EChessPlayerPosition InPosition, AActor* InPerformer);

	virtual void BeginTurn();
	virtual void EndTurn();

	UFUNCTION(BlueprintPure, Category="Chess Participant")
	EChessPlayerPosition GetPosition() const { return Position; }

	UFUNCTION(BlueprintPure, Category="Chess Participant")
	bool IsTurnActive() const { return bIsTurnActive; }

	AChessMatch* GetMatch() const { return Match; }
	AActor* GetPerformer() const { return Performer.Get(); }

protected:
	UPROPERTY(Transient)
	TObjectPtr<AChessMatch> Match;

	TWeakObjectPtr<AActor> Performer;
	EChessPlayerPosition Position = EChessPlayerPosition::PlayerA;
	bool bIsTurnActive = false;
};
