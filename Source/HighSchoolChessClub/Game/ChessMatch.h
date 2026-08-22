#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChessCoreTypes.h"
#include "Game/ChessMatchSettings.h"
#include "Game/ChessParticipantTypes.h"
#include "ChessMatch.generated.h"

class AChessDesk;
class UChessClockComponent;
class UChessGameState;
class UChessHumanParticipant;
class UChessParticipant;

UENUM(BlueprintType)
enum class EChessMatchState : uint8
{
	WaitingForPlayers,
	MatchSetup,
	Playing,
	Finished
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChessMatchStateChanged, EChessMatchState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChessBoardStateChanged, UChessGameState*, GameState);

UCLASS(Blueprintable)
class HIGHSCHOOLCHESSCLUB_API AChessMatch : public AActor
{
	GENERATED_BODY()

public:
	AChessMatch();

#pragma region Board
	UFUNCTION(BlueprintPure, Category="Chess Match")
	AChessDesk* GetDesk() const { return Desk; }

	bool TrySubmitMove(UChessParticipant* Participant, const FChessCoreMove& Move);
	bool TrySubmitMoveUci(UChessParticipant* Participant, const FString& UciMove);
	void GetLegalMovesFrom(FIntPoint Square, TArray<FChessCoreMove>& OutMoves) const;

	/**
	 * @param Square 체스 좌표
	 * @param OutPiece 반환 받을 ref
	 * @return 해당 좌표에 피스가 존재하면 true, 없다면 false
	 */
	bool GetPieceAtSquare(FIntPoint Square, FChessCorePiece& OutPiece) const;

	UFUNCTION(BlueprintPure, Category="Chess Match")
	FString GetFen() const;

	UFUNCTION(BlueprintPure, Category="Chess Match")
	EChessCorePieceColor GetSideToMove() const;

	UFUNCTION(BlueprintPure, Category="Chess Match")
	UChessGameState* GetChessState() const { return ChessState; }

	UFUNCTION(BlueprintCallable, Category="Chess Match")
	void SetupInitialPosition();

	UPROPERTY(BlueprintAssignable, Category="ChessMatch")
	FOnChessBoardStateChanged OnBoardStateChanged;
#pragma endregion

#pragma region Participants
	UFUNCTION(BlueprintPure, Category="Chess Match")
	UChessParticipant* GetParticipant(EChessPlayerPosition Position) const;

	UChessHumanParticipant* GetHumanParticipant(EChessPlayerPosition Position) const;

	UFUNCTION(BlueprintCallable, Category="Chess Match")
	UChessParticipant* RegisterParticipant(EChessPlayerPosition Position, AActor* Performer);

	UFUNCTION(BlueprintCallable, Category="Chess Match")
	bool UnregisterParticipant(UChessParticipant* Participant);

	/*
	 * 등록된 HumanParticipant의 Pawn이 Unpossess되었음을 알린다.
	 * 설정 중이었다면 플레이어 대기 상태로 돌아가고, 대국 중이었다면 매치를 종료한다.
	 */
	void NotifyHumanPlayerUnpossessed(UChessHumanParticipant* Participant);

	UFUNCTION(BlueprintPure, Category="Chess Match")
	EChessCorePieceColor GetPlayerColor(EChessPlayerPosition Position) const;

	UFUNCTION(BlueprintPure, Category="ChessMatch")
	EChessCorePieceColor GetHumanPlayerColor() const;
#pragma endregion

#pragma region Match
	/* HumanParticipant의 Pawn Possess가 완료되면 매치 설정 단계로 전환한다. */
	void EnterMatchSetup();

	UFUNCTION(BlueprintCallable, Category="Chess Match")
	void RequestStartMatch();

	UFUNCTION(BlueprintPure, Category="Chess Match")
	EChessMatchState GetMatchState() const { return MatchState; }

	UPROPERTY(BlueprintAssignable, Category="Chess Match")
	FOnChessMatchStateChanged OnMatchStateChanged;
#pragma endregion

#pragma region Settings
	UFUNCTION(BlueprintCallable, Category="Chess Match")
	void SetMatchSettings(const FChessMatchSettings& InSettings);

	UFUNCTION(BlueprintPure, Category="Chess Match")
	FChessMatchSettings GetMatchSettings() const { return MatchSettings; }
#pragma endregion

#pragma region Time
	UFUNCTION(BlueprintPure, Category="Chess Match")
	UChessClockComponent* GetChessClock() const { return ChessClock; }
#pragma endregion

protected:
#pragma region Actor
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
#pragma endregion

#pragma region Components
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Chess Match")
	TObjectPtr<AChessDesk> Desk;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chess Match")
	TObjectPtr<UChessClockComponent> ChessClock;
#pragma endregion

#pragma region Participants
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Chess Match")
	EChessCorePieceColor PlayerAColor = EChessCorePieceColor::White;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Chess Match")
	TSubclassOf<UChessParticipant> PlayerAParticipantClass;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Chess Match")
	TSubclassOf<UChessParticipant> PlayerBParticipantClass;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Chess Match")
	TObjectPtr<AActor> PlayerAPerformer;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Chess Match")
	TObjectPtr<AActor> PlayerBPerformer;
#pragma endregion

private:
#pragma region Runtime State
	UPROPERTY(Transient)
	TObjectPtr<UChessGameState> ChessState;

	UPROPERTY(Transient)
	TObjectPtr<UChessParticipant> PlayerAParticipant;

	UPROPERTY(Transient)
	TObjectPtr<UChessParticipant> PlayerBParticipant;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category="Chess Match", meta=(AllowPrivateAccess="true"))
	FChessMatchSettings MatchSettings;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category="Chess Match", meta=(AllowPrivateAccess="true"))
	EChessMatchState MatchState = EChessMatchState::WaitingForPlayers;
#pragma endregion

#pragma region Match Flow
	/* BeginPlay 시 미리 지정된 Performer를 각 좌석의 Participant로 등록한다. */
	void RegisterConfiguredParticipants();

	/*
	 * Participant 등록 상태가 바뀐 뒤 매치 상태를 동기화한다.
	 * 대국 전 두 좌석 중 하나라도 비어 있으면 플레이어 대기 상태로 되돌린다.
	 */
	void RefreshParticipantState();

	/* 양쪽 턴을 끝내고 체스 시계를 멈춘 뒤 매치를 종료 상태로 전환한다. */
	void FinishMatch();

	/* 상태가 실제로 변경될 때만 새 상태를 저장하고 OnMatchStateChanged를 전파한다. */
	void SetMatchState(EChessMatchState NewState);

	/* 현재 수를 둘 색에 해당하는 Participant의 턴과 체스 시계를 시작한다. */
	void BeginCurrentTurn();
#pragma endregion

#pragma region Board
	void RebuildDeskFromState();
	UChessParticipant* GetParticipantForColor(EChessCorePieceColor Color) const;
#pragma endregion

#pragma region Time
	UFUNCTION()
	void HandleTimeExpired(EChessPlayerPosition Position);
#pragma endregion
};
