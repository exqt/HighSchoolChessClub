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
	bTweenRotation = false;
	PositionTweenElapsedTime = 0.0f;
	PositionTweenDuration = TweenDuration;
	bIsPositionTweening = true;
	SetComponentTickEnabled(true);
}

void UPositionTweenComponent::TweenToTransform(const FVector TargetWorldPosition, const FRotator TargetWorldRotation, const float Duration)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const float TweenDuration = Duration >= 0.0f ? Duration : DefaultPositionTweenDuration;
	if (TweenDuration <= 0.0f)
	{
		Owner->SetActorLocationAndRotation(TargetWorldPosition, TargetWorldRotation);
		StopPositionTween();
		return;
	}

	TweenStartPosition = Owner->GetActorLocation();
	TweenTargetPosition = TargetWorldPosition;
	TweenStartRotation = Owner->GetActorQuat();
	TweenTargetRotation = TargetWorldRotation.Quaternion();
	bTweenRotation = true;
	PositionTweenElapsedTime = 0.0f;
	PositionTweenDuration = TweenDuration;
	bIsPositionTweening = true;
	SetComponentTickEnabled(true);
}

void UPositionTweenComponent::StopPositionTween(const bool bSnapToTarget)
{
	if (bSnapToTarget && bIsPositionTweening)
	{
		if (bTweenRotation)
		{
			GetOwner()->SetActorLocationAndRotation(TweenTargetPosition, TweenTargetRotation);
		}
		else
		{
			GetOwner()->SetActorLocation(TweenTargetPosition);
		}
	}

	bIsPositionTweening = false;
	bTweenRotation = false;
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
	const FVector CurrentPosition = FMath::Lerp(TweenStartPosition, TweenTargetPosition, EasedAlpha);
	if (bTweenRotation)
	{
		GetOwner()->SetActorLocationAndRotation(CurrentPosition, FQuat::Slerp(TweenStartRotation, TweenTargetRotation, EasedAlpha));
	}
	else
	{
		GetOwner()->SetActorLocation(CurrentPosition);
	}

	if (Alpha >= 1.0f)
	{
		StopPositionTween(true);
	}
}
