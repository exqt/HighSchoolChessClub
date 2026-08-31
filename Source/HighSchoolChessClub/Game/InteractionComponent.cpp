// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionComponent.h"

#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "Interactable.h"
#include "InteractionWidgetComponent.h"

namespace
{
	constexpr ECollisionChannel ChairDeskObjectChannel = ECC_GameTraceChannel1;
}


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
void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
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
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return SetCurrentInteractable(nullptr);

	const AController* Controller = OwnerPawn->GetController();
	if (!Controller) return SetCurrentInteractable(nullptr);

	FVector ViewLocation;
	FRotator ViewRotation;

	Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);

	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerPawn);
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ChairDeskObjectChannel);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	GetWorld()->OverlapMultiByObjectType(OverlapResults, ViewLocation, FQuat::Identity, ObjectQueryParams, FCollisionShape::MakeSphere(InteractionDistance), Params);

	AActor* BestInteractable = nullptr;
	float BestDot = MinimumViewDot;
	float BestDistance = InteractionDistance;
	TSet<AActor*> CheckedActors;

	for (const FOverlapResult& Overlap : OverlapResults)
	{
		AActor* Candidate = Overlap.GetActor();
		if (!IsValid(Candidate) || CheckedActors.Contains(Candidate)) continue;
		CheckedActors.Add(Candidate);
		if (!Candidate->GetClass()->ImplementsInterface(UInteractable::StaticClass()) || !IInteractable::Execute_CanInteract(Candidate, OwnerPawn)) continue;

		const UInteractionWidgetComponent* InteractionWidget = Candidate->FindComponentByClass<UInteractionWidgetComponent>();
		const FVector TargetLocation = InteractionWidget ? InteractionWidget->GetComponentLocation() : Candidate->GetActorLocation();
		const FVector DirectionToCandidate = TargetLocation - ViewLocation;
		const float Distance = DirectionToCandidate.Size();
		if (Distance <= UE_SMALL_NUMBER || Distance > InteractionDistance) continue;

		const float ViewDot = FVector::DotProduct(ViewRotation.Vector(), DirectionToCandidate / Distance);
		if (ViewDot < MinimumViewDot) continue;
		if (ViewDot > BestDot || FMath::IsNearlyEqual(ViewDot, BestDot) && Distance < BestDistance)
		{
			BestInteractable = Candidate;
			BestDot = ViewDot;
			BestDistance = Distance;
		}
	}

	SetCurrentInteractable(BestInteractable);
}

void UInteractionComponent::SetCurrentInteractable(AActor* NewCurrentInteractable)
{
	if (CurrentInteractable == NewCurrentInteractable) return;
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	
	if (CurrentInteractable != nullptr)
	{
		if (UInteractionWidgetComponent* InteractionWidget = CurrentInteractable->FindComponentByClass<UInteractionWidgetComponent>()) InteractionWidget->SetInteractionHovered(false);
		IInteractable::Execute_OnInteractionUnhover(CurrentInteractable, OwnerPawn);
		OnInteractionUnhover.Broadcast(CurrentInteractable);
	}

	CurrentInteractable = NewCurrentInteractable;

	if (CurrentInteractable != nullptr)
	{
		if (UInteractionWidgetComponent* InteractionWidget = CurrentInteractable->FindComponentByClass<UInteractionWidgetComponent>()) InteractionWidget->SetInteractionHovered(true);
		IInteractable::Execute_OnInteractionHover(CurrentInteractable, OwnerPawn);
		OnInteractionHover.Broadcast(CurrentInteractable);
	}
}
