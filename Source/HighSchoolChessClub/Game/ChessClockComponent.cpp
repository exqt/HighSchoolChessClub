#include "Game/ChessClockComponent.h"

UChessClockComponent::UChessClockComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UChessClockComponent::ResetClock()
{
	StopClock();
	PlayerARemainingTime = InitialTime;
	PlayerBRemainingTime = InitialTime;
	PlayerATimeAccumulator = 0.0f;
	PlayerBTimeAccumulator = 0.0f;
	OnTimeChanged.Broadcast(EChessPlayerPosition::PlayerA, PlayerARemainingTime);
	OnTimeChanged.Broadcast(EChessPlayerPosition::PlayerB, PlayerBRemainingTime);
}

void UChessClockComponent::StartClock(const EChessPlayerPosition StartingPosition)
{
	ActivePlayer = StartingPosition;
	bIsRunning = true;
	SetComponentTickEnabled(true);
}

void UChessClockComponent::StopClock()
{
	bIsRunning = false;
	SetComponentTickEnabled(false);
}

void UChessClockComponent::SetActivePlayer(const EChessPlayerPosition Position)
{
	ActivePlayer = Position;
}

void UChessClockComponent::ConfigureClock(const int32 InInitialTime, const int32 InIncrementTime)
{
	InitialTime = FMath::Max(0, InInitialTime);
	IncrementTime = FMath::Max(0, InIncrementTime);
}

void UChessClockComponent::ApplyIncrement(const EChessPlayerPosition Position)
{
	if (IncrementTime <= 0)
	{
		return;
	}

	int32& RemainingTime = Position == EChessPlayerPosition::PlayerA
		? PlayerARemainingTime
		: PlayerBRemainingTime;
	RemainingTime = static_cast<int32>(FMath::Min<int64>(
		MAX_int32,
		static_cast<int64>(RemainingTime) + IncrementTime));
	OnTimeChanged.Broadcast(Position, RemainingTime);
}

int32 UChessClockComponent::GetRemainingTime(const EChessPlayerPosition Position) const
{
	return Position == EChessPlayerPosition::PlayerA
		? PlayerARemainingTime
		: PlayerBRemainingTime;
}

void UChessClockComponent::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsRunning)
	{
		return;
	}

	float& TimeAccumulator = ActivePlayer == EChessPlayerPosition::PlayerA
		? PlayerATimeAccumulator
		: PlayerBTimeAccumulator;
	TimeAccumulator += DeltaTime;

	const int32 ElapsedTenths = FMath::FloorToInt(TimeAccumulator * 10.0f);
	if (ElapsedTenths <= 0)
	{
		return;
	}
	TimeAccumulator -= static_cast<float>(ElapsedTenths) * 0.1f;

	int32& RemainingTime = ActivePlayer == EChessPlayerPosition::PlayerA
		? PlayerARemainingTime
		: PlayerBRemainingTime;
	RemainingTime = FMath::Max(0, RemainingTime - ElapsedTenths);
	OnTimeChanged.Broadcast(ActivePlayer, RemainingTime);

	if (RemainingTime == 0)
	{
		const EChessPlayerPosition ExpiredPlayer = ActivePlayer;
		StopClock();
		OnTimeExpired.Broadcast(ExpiredPlayer);
	}
}
