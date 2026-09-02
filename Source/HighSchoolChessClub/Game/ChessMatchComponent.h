#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ChessCoreTypes.h"
#include "Game/ChessMatchSettings.h"
#include "Game/ChessGameTypes.h"
#include "ChessMatchComponent.generated.h"

class AChessDesk;
class ANPCBase;
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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChessParticipantRegistered, UChessParticipant*, Participant);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChessParticipantUnregistered, UChessParticipant*, Participant);

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Chess))
class HIGHSCHOOLCHESSCLUB_API UChessMatchComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UChessMatchComponent();

#pragma region Board
	UFUNCTION(BlueprintPure, Category="ChessMatchComponent")
	AChessDesk* GetDesk() const;

	bool TrySubmitMove(UChessParticipant* Participant, const FChessCoreMove& Move, EChessMoveVisualMode VisualMode = EChessMoveVisualMode::Tween);
	bool TrySubmitMoveUci(UChessParticipant* Participant, const FString& UciMove);
	bool FindLegalMoveUci(const FString& UciMove, FChessCoreMove& OutMove) const;
	void GetLegalMovesFrom(FIntPoint Square, TArray<FChessCoreMove>& OutMoves) const;

	/**
	 * @param Square 체스 좌표
	 * @param OutPiece 반환 받을 ref
	 * @return 해당 좌표에 피스가 존재하면 true, 없다면 false
	 */
	bool GetPieceAtSquare(FIntPoint Square, FChessCorePiece& OutPiece) const;

	UFUNCTION(BlueprintPure, Category="ChessMatchComponent")
	FString GetFen() const;

	UFUNCTION(BlueprintPure, Category="ChessMatchComponent")
	EChessCorePieceColor GetSideToMove() const;

	UFUNCTION(BlueprintPure, Category="ChessMatchComponent")
	UChessGameState* GetChessState() const { return ChessState; }

	UFUNCTION(BlueprintCallable, Category="ChessMatchComponent")
	void SetupInitialPosition();

	UFUNCTION(BlueprintCallable, Category="ChessMatchComponent")
	bool SetupPositionFromFen(const FString& Fen);

	UPROPERTY(BlueprintAssignable, Category="ChessMatchComponent")
	FOnChessBoardStateChanged OnBoardStateChanged;
#pragma endregion

#pragma region Participants
	UFUNCTION(BlueprintPure, Category="ChessMatchComponent")
	UChessParticipant* GetParticipant(EChessPlayerPosition Position) const;

	UFUNCTION(BlueprintPure, Category="ChessMatchComponent")
	AActor* GetPerformer(EChessPlayerPosition Position) const;

	UFUNCTION(BlueprintPure, Category="ChessMatchComponent")
	TArray<ANPCBase*> GetNPCPerformers() const;

	UChessHumanParticipant* GetHumanParticipant(EChessPlayerPosition Position) const;

	UFUNCTION(BlueprintCallable, Category="ChessMatchComponent")
	UChessParticipant* RegisterParticipant(EChessPlayerPosition Position, AActor* Performer, TSubclassOf<UChessParticipant> ParticipantClass);

	UPROPERTY(BlueprintAssignable, Category="ChessMatchComponent")
	FOnChessParticipantRegistered OnParticipantRegistered;

	UFUNCTION(BlueprintCallable, Category="ChessMatchComponent")
	bool UnregisterParticipant(UChessParticipant* Participant);

	UPROPERTY(BlueprintAssignable, Category="ChessMatchComponent")
	FOnChessParticipantUnregistered OnParticipantUnregistered;

	UFUNCTION(BlueprintCallable, Category="ChessMatchComponent")
	UChessParticipant* RegisterNPCParticipant(EChessPlayerPosition Position, ANPCBase* NPCPerformer);

	UFUNCTION(BlueprintCallable, Category="ChessMatchComponent")
	bool UnregisterNPCParticipant(EChessPlayerPosition Position, ANPCBase* NPCPerformer);

	/*
	 * 등록된 HumanParticipant의 Pawn이 Unpossess되었음을 알린다.
	 * 설정 중이었다면 플레이어 대기 상태로 돌아가고, 대국 중이었다면 매치를 종료한다.
	 */
	void NotifyHumanPlayerUnpossessed(UChessHumanParticipant* Participant);

	UFUNCTION(BlueprintPure, Category="ChessMatchComponent")
	EChessCorePieceColor GetPlayerColor(EChessPlayerPosition Position) const;

	UFUNCTION(BlueprintPure, Category="ChessMatchComponent")
	EChessCorePieceColor GetHumanPlayerColor() const;

	UFUNCTION(BlueprintPure, Category="ChessMatchComponent")
	EChessPlayerPosition GetHumanPlayerPosition() const { return EChessPlayerPosition::PlayerA; }

#pragma endregion

#pragma region Match
	/* HumanParticipant의 Pawn Possess가 완료되면 매치 설정 단계로 전환한다. */
	void EnterMatchSetup();

	UFUNCTION(BlueprintCallable, Category="ChessMatchComponent")
	void RequestStartMatch();

	UFUNCTION(BlueprintPure, Category="ChessMatchComponent")
	EChessMatchState GetMatchState() const { return MatchState; }

	UPROPERTY(BlueprintAssignable, Category="ChessMatchComponent")
	FOnChessMatchStateChanged OnMatchStateChanged;
#pragma endregion

#pragma region Settings
	UFUNCTION(BlueprintCallable, Category="ChessMatchComponent")
	void SetMatchSettings(const FChessMatchSettings& InSettings);

	UFUNCTION(BlueprintPure, Category="ChessMatchComponent")
	FChessMatchSettings GetMatchSettings() const { return MatchSettings; }
#pragma endregion

#pragma region Time
	UFUNCTION(BlueprintPure, Category="ChessMatchComponent")
	UChessClockComponent* GetChessClock() const;
#pragma endregion

protected:
#pragma region Component
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
#pragma endregion

#pragma region Debug
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="ChessMatchComponent")
	bool bUseDebugPosition = false;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="ChessMatchComponent", meta=(EditCondition="bUseDebugPosition"))
	FString DebugPositionFen = TEXT("8/7Q/2N5/8/8/k1pp4/2qp4/K7 b - - 1 1");
#pragma endregion

#pragma region Participants
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="ChessMatchComponent")
	EChessCorePieceColor PlayerAColor = EChessCorePieceColor::White;

#pragma endregion

private:
#pragma region Runtime State
	UPROPERTY(Transient)
	TObjectPtr<UChessGameState> ChessState;

	UPROPERTY(Transient)
	TObjectPtr<UChessParticipant> PlayerAParticipant;

	UPROPERTY(Transient)
	TObjectPtr<UChessParticipant> PlayerBParticipant;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category="ChessMatchComponent", meta=(AllowPrivateAccess="true"))
	FChessMatchSettings MatchSettings;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category="ChessMatchComponent", meta=(AllowPrivateAccess="true"))
	EChessMatchState MatchState = EChessMatchState::WaitingForPlayers;
#pragma endregion

#pragma region Match Flow
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
