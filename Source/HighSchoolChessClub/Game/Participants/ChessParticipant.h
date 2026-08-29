#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Game/ChessGameTypes.h"
#include "ChessParticipant.generated.h"

class AActor;
class AChessMatch;

/* 실제 ChessMatch에 참여하는 주체 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class HIGHSCHOOLCHESSCLUB_API UChessParticipant : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(AChessMatch* InMatch, EChessPlayerPosition InPosition, AActor* InPerformer);

	UFUNCTION(BlueprintPure, Category="Chess Participant")
	EChessPlayerPosition GetPosition() const { return Position; }

	AChessMatch* GetMatch() const { return Match; }
	AActor* GetPerformer() const { return Performer.Get(); }

#pragma region Turn
	virtual void BeginTurn();
	virtual void EndTurn();

	UFUNCTION(BlueprintPure, Category="Chess Participant")
	bool IsTurnActive() const { return bIsTurnActive; }
#pragma endregion

protected:
#pragma region Runtime State
	UPROPERTY(Transient)
	TObjectPtr<AChessMatch> Match;

	TWeakObjectPtr<AActor> Performer;
	EChessPlayerPosition Position = EChessPlayerPosition::PlayerA;
	bool bIsTurnActive = false;
#pragma endregion
};
