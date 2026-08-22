#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Game/ChessParticipantTypes.h"
#include "ChessClockComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnChessClockTimeChanged,
	EChessPlayerPosition, Position,
	int32, RemainingTime);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnChessClockExpired,
	EChessPlayerPosition, Position);

UCLASS(ClassGroup=(Chess), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class HIGHSCHOOLCHESSCLUB_API UChessClockComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UChessClockComponent();

	/** 두 플레이어의 시간을 InitialTime으로 되돌리고 시계를 정지한다. */
	UFUNCTION(BlueprintCallable, Category="Chess Clock")
	void ResetClock();

	UFUNCTION(BlueprintCallable, Category="Chess Clock")
	void StartClock(EChessPlayerPosition StartingPosition);

	UFUNCTION(BlueprintCallable, Category="Chess Clock")
	void StopClock();

	UFUNCTION(BlueprintCallable, Category="Chess Clock")
	void SetActivePlayer(EChessPlayerPosition Position);

	UFUNCTION(BlueprintCallable, Category="Chess Clock")
	void ConfigureClock(int32 InInitialTime, int32 InIncrementTime);

	UFUNCTION(BlueprintCallable, Category="Chess Clock")
	void ApplyIncrement(EChessPlayerPosition Position);

	UFUNCTION(BlueprintPure, Category="Chess Clock")
	int32 GetRemainingTime(EChessPlayerPosition Position) const;

	UFUNCTION(BlueprintPure, Category="Chess Clock")
	EChessPlayerPosition GetActivePlayer() const { return ActivePlayer; }

	UFUNCTION(BlueprintPure, Category="Chess Clock")
	bool IsClockRunning() const { return bIsRunning; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chess Clock", meta=(ClampMin="0"))
	int32 InitialTime = 6000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chess Clock", meta=(ClampMin="0"))
	int32 IncrementTime = 50;

	UPROPERTY(BlueprintAssignable, Category="Chess Clock")
	FOnChessClockTimeChanged OnTimeChanged;

	UPROPERTY(BlueprintAssignable, Category="Chess Clock")
	FOnChessClockExpired OnTimeExpired;

protected:
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category="Chess Clock", meta=(AllowPrivateAccess="true"))
	int32 PlayerARemainingTime = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category="Chess Clock", meta=(AllowPrivateAccess="true"))
	int32 PlayerBRemainingTime = 0;

	EChessPlayerPosition ActivePlayer = EChessPlayerPosition::PlayerA;
	float PlayerATimeAccumulator = 0.0f;
	float PlayerBTimeAccumulator = 0.0f;
	bool bIsRunning = false;
};
