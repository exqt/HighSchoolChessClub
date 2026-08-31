#include "InteractionWidgetComponent.h"

#include "Game/Interactable.h"
#include "UI/Widgets/InteractionActionWidget.h"

UInteractionWidgetComponent::UInteractionWidgetComponent()
{
	SetWidgetSpace(EWidgetSpace::Screen);
	SetDrawSize(FVector2D(310.0, 78.0));
	SetPivot(FVector2D(0.5, 0.5));
	SetRelativeScale3D(FVector(0.2));
	SetBlendMode(EWidgetBlendMode::Masked);
	SetTickWhenOffscreen(false);
	SetVisibility(false);
}

void UInteractionWidgetComponent::BeginPlay()
{
	Super::BeginPlay();
	InitWidget();
}

void UInteractionWidgetComponent::SetInteractionHovered(const bool bIsHovered)
{
	if (bIsHovered) UpdateInteractionText();
	SetVisibility(bIsHovered);
}

void UInteractionWidgetComponent::UpdateInteractionText()
{
	UInteractionActionWidget* InteractionActionWidget = Cast<UInteractionActionWidget>(GetUserWidgetObject());
	if (!InteractionActionWidget) return;
	InteractionActionWidget->UpdateInteractionText(IInteractable::Execute_GetInteractionName(GetOwner()));
}
