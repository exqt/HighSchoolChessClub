#include "Game/PositionTweenComponent.h"

#include "GameFramework/Actor.h"

UPositionTweenComponent::UPositionTweenComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UPositionTweenComponent::TweenToPosition(const FVector TargetWorldPosition, const float Duration)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const float TweenDuration = Duration >= 0.0f ? Duration : DefaultPositionTweenDuration;
	if (TweenDuration <= 0.0f)
	{
		Owner->SetActorLocation(TargetWorldPosition);
		StopPositionTween();
		return;
	}

	TweenStartPosition = Owner->GetActorLocation();
	TweenTargetPosition = TargetWorldPosition;
	PositionTweenElapsedTime = 0.0f;
	PositionTweenDuration = TweenDuration;
	bIsPositionTweening = true;
	SetComponentTickEnabled(true);
}

void UPositionTweenComponent::StopPositionTween(const bool bSnapToTarget)
{
	if (bSnapToTarget && bIsPositionTweening)
	{
		GetOwner()->SetActorLocation(TweenTargetPosition);
	}

	bIsPositionTweening = false;
	SetComponentTickEnabled(false);
}

void UPositionTweenComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsPositionTweening)
	{
		return;
	}

	PositionTweenElapsedTime += DeltaTime;
	const float Alpha = FMath::Clamp(PositionTweenElapsedTime / PositionTweenDuration, 0.0f, 1.0f);
	const float EasedAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, PositionTweenEaseExponent);
	GetOwner()->SetActorLocation(FMath::Lerp(TweenStartPosition, TweenTargetPosition, EasedAlpha));

	if (Alpha >= 1.0f)
	{
		GetOwner()->SetActorLocation(TweenTargetPosition);
		StopPositionTween();
	}
}
