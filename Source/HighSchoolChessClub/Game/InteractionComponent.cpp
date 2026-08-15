// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionComponent.h"

#include "IMediaControls.h"
#include "Interactable.h"


// Sets default values for this component's properties
UInteractionComponent::UInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UInteractionComponent::TickComponent(
	float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CheckInteractable();	
}

TScriptInterface<IInteractable> UInteractionComponent::GetCurrentInteractable()
{
	return CurrentInteractable;
}

bool UInteractionComponent::TryInteract()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !IsValid(CurrentInteractable))
	{
		return false;
	}

	if (!IInteractable::Execute_CanInteract(CurrentInteractable, OwnerPawn))
	{
		return false;
	}

	IInteractable::Execute_Interact(CurrentInteractable, OwnerPawn);
	return true;
}

void UInteractionComponent::CheckInteractable()
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return SetCurrentInteractable(nullptr);

	const AController* Controller = OwnerPawn->GetController();
	if (!Controller) return SetCurrentInteractable(nullptr);

	FVector ViewLocation;
	FRotator ViewRotation;

	Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);

	const FVector TraceStart = ViewLocation;
	const FVector TraceEnd =
		TraceStart + ViewRotation.Vector() * InteractionDistance;

	FHitResult Hit;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerPawn);

	const bool IsHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		Params
	);
	
	if (!IsHit) return SetCurrentInteractable(nullptr);

	AActor* HitActor = Hit.GetActor();

	if (HitActor == nullptr || !HitActor->GetClass()->ImplementsInterface(UInteractable::StaticClass())) return SetCurrentInteractable(nullptr);

	return SetCurrentInteractable(HitActor);
}

void UInteractionComponent::SetCurrentInteractable(AActor* NewCurrentInteractable)
{
	if (CurrentInteractable == NewCurrentInteractable) return;
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	
	if (CurrentInteractable != nullptr)
	{
		IInteractable::Execute_OnInteractionUnhover(CurrentInteractable, OwnerPawn);
		OnInteractionUnhover.Broadcast(CurrentInteractable);
	}

	CurrentInteractable = NewCurrentInteractable;

	if (CurrentInteractable != nullptr)
	{
		IInteractable::Execute_OnInteractionHover(CurrentInteractable, OwnerPawn);
		OnInteractionHover.Broadcast(CurrentInteractable);
	}
}
