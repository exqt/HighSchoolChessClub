#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Game/ChessGameTypes.h"
#include "ChessParticipant.generated.h"

class AActor;
class UChessMatchComponent;

/* 실제 ChessMatch에 참여하는 주체 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class HIGHSCHOOLCHESSCLUB_API UChessParticipant : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UChessMatchComponent* InMatch, EChessPlayerPosition InPosition, AActor* InPerformer);

	UFUNCTION(BlueprintPure, Category="Chess Participant")
	EChessPlayerPosition GetPosition() const { return Position; }

	UFUNCTION(BlueprintPure, Category="ChessParticipant")
	UChessMatchComponent* GetMatch() const { return Match; }

	UFUNCTION(BlueprintPure, Category="ChessParticipant")
	bool IsWinner(EChessCoreGameResult GameResult) const;

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
	TObjectPtr<UChessMatchComponent> Match;

	TWeakObjectPtr<AActor> Performer;
	EChessPlayerPosition Position = EChessPlayerPosition::PlayerA;
	bool bIsTurnActive = false;
#pragma endregion
};
