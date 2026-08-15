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
	// Sets default values for this component's properties
	UInteractionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	UPROPERTY(BlueprintAssignable)
	FOnInteractionHoverSignature OnInteractionHover;
	
	UPROPERTY(BlueprintAssignable)
	FOnInteractionHoverSignature OnInteractionUnhover;
	
	UFUNCTION(BlueprintCallable)
	TScriptInterface<IInteractable> GetCurrentInteractable();

	/** Executes the interaction on the actor currently under the player's crosshair. */
	UFUNCTION(BlueprintCallable, Category="Interaction")
	bool TryInteract();
	
private:
	/**
	 * Viewport로부터 상호작용 가능한 Actor가 있는지 검사
	 */
	UFUNCTION(BlueprintCallable, Category="Interaction")
	void CheckInteractable();

	UPROPERTY(EditAnywhere, Category="Interaction", meta=(ClampMin="0.0", ClampMax="300.0"))
	float InteractionDistance = 200.0f;
	
	UPROPERTY(Transient)
	TObjectPtr<AActor> CurrentInteractable;
	
	void SetCurrentInteractable(AActor* NewCurrentInteractable);	
};
