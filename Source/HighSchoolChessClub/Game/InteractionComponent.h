// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class IInteractable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionHoverSignature, TScriptInterface<IInteractable>, Interactable);


/**
 * IInteractable과 상호작용하기 위한 컴포넌트, 플레이어에게 부착
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HIGHSCHOOLCHESSCLUB_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UPROPERTY(BlueprintAssignable)
	FOnInteractionHoverSignature OnInteractionHover;
	
	UPROPERTY(BlueprintAssignable)
	FOnInteractionHoverSignature OnInteractionUnhover;
	
	UFUNCTION(BlueprintCallable)
	TScriptInterface<IInteractable> GetCurrentInteractable();

	UFUNCTION(BlueprintCallable, Category="Interaction")
	bool TryInteract();
	
private:
	/** 카메라 방향과 가까운 상호작용 가능한 Actor를 검사 */
	UFUNCTION(BlueprintCallable, Category="Interaction")
	void CheckInteractable();

	UPROPERTY(EditAnywhere, Category="Interaction Component", meta=(ClampMin="0.0", Units="cm"))
	float InteractionDistance = 200.0f;

	UPROPERTY(EditAnywhere, Category="Interaction Component", meta=(ClampMin="-1.0", ClampMax="1.0"))
	float MinimumViewDot = 0.8f;
	
	UPROPERTY(Transient)
	TObjectPtr<AActor> CurrentInteractable;
	
	void SetCurrentInteractable(AActor* NewCurrentInteractable);	
};
