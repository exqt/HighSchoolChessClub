#pragma once

#include "CoreMinimal.h"
#include "ChessCoreTypes.h"
#include "Game/ChessParticipantTypes.h"
#include "GameFramework/Actor.h"
#include "ChessMatch.generated.h"

class AChessDesk;
class UChessGameState;
class UChessHumanParticipant;
class UChessParticipant;

UCLASS(Blueprintable)
class HIGHSCHOOLCHESSCLUB_API AChessMatch : public AActor
{
	GENERATED_BODY()

public:
	AChessMatch();

	UFUNCTION(BlueprintPure, Category="Chess Match")
	AChessDesk* GetDesk() const { return Desk; }

	UFUNCTION(BlueprintPure, Category="Chess Match|Participants")
	UChessParticipant* GetParticipant(EChessPlayerPosition Position) const;

	UChessHumanParticipant* GetHumanParticipant(EChessPlayerPosition Position) const;

	bool TrySubmitMove(UChessParticipant* Participant, const FChessCoreMove& Move);
	bool TrySubmitMoveUci(UChessParticipant* Participant, const FString& UciMove);
	void GetLegalMovesFrom(FIntPoint Square, TArray<FChessCoreMove>& OutMoves) const;
	bool GetPieceAtSquare(FIntPoint Square, FChessCorePiece& OutPiece) const;

	UFUNCTION(BlueprintPure, Category="Chess Match|Game")
	FString GetFen() const;

	UFUNCTION(BlueprintPure, Category="Chess Match|Game")
	EChessCorePieceColor GetSideToMove() const;

	UFUNCTION(BlueprintPure, Category="Chess Match|Participants")
	EChessCorePieceColor GetPlayerColor(EChessPlayerPosition Position) const;

	UFUNCTION(BlueprintPure, Category="Chess Match|Game")
	UChessGameState* GetChessState() const { return ChessState; }

	UFUNCTION(BlueprintCallable, Category="Chess Match|Game")
	void SetupInitialPosition();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Chess Match")
	TObjectPtr<AChessDesk> Desk;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Chess Match|Participants")
	EChessCorePieceColor PlayerAColor = EChessCorePieceColor::White;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Chess Match|Participants")
	TSubclassOf<UChessParticipant> PlayerAParticipantClass;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Chess Match|Participants")
	TSubclassOf<UChessParticipant> PlayerBParticipantClass;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Chess Match|Participants")
	TObjectPtr<AActor> PlayerAPerformer;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Chess Match|Participants")
	TObjectPtr<AActor> PlayerBPerformer;

	UFUNCTION(BlueprintImplementableEvent, Category="Chess Match|Game")
	void OnMoveApplied(const FChessCoreMove& Move);

private:
	UPROPERTY(Transient)
	TObjectPtr<UChessGameState> ChessState;

	UPROPERTY(Transient)
	TObjectPtr<UChessParticipant> PlayerAParticipant;

	UPROPERTY(Transient)
	TObjectPtr<UChessParticipant> PlayerBParticipant;

	void CreateParticipants();
	void BeginCurrentTurn();
	void RebuildDeskFromState();
	UChessParticipant* GetParticipantForColor(EChessCorePieceColor Color) const;
};
