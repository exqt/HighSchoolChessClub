#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PositionTweenComponent.generated.h"

UCLASS(ClassGroup=(Tween), meta=(BlueprintSpawnableComponent))
class HIGHSCHOOLCHESSCLUB_API UPositionTweenComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPositionTweenComponent();

	UFUNCTION(BlueprintCallable, Category="Tween|Position")
	void TweenToPosition(FVector TargetWorldPosition, float Duration = -1.0f);

	void TweenToTransform(FVector TargetWorldPosition, FRotator TargetWorldRotation, float Duration = -1.0f);

	UFUNCTION(BlueprintCallable, Category="Tween|Position")
	void StopPositionTween(bool bSnapToTarget = false);

	UFUNCTION(BlueprintPure, Category="Tween|Position")
	bool IsPositionTweening() const { return bIsPositionTweening; }

protected:
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tween|Position", meta=(ClampMin="0.0"))
	float DefaultPositionTweenDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tween|Position", meta=(ClampMin="1.0"))
	float PositionTweenEaseExponent = 2.0f;

private:
	FVector TweenStartPosition = FVector::ZeroVector;
	FVector TweenTargetPosition = FVector::ZeroVector;
	FQuat TweenStartRotation = FQuat::Identity;
	FQuat TweenTargetRotation = FQuat::Identity;
	float PositionTweenElapsedTime = 0.0f;
	float PositionTweenDuration = 0.0f;
	bool bIsPositionTweening = false;
	bool bTweenRotation = false;
};
