#include "UI/Widgets/SaveSlots/CCSaveSlotItem.h"

#include "Components/WidgetSwitcher.h"

void UCCSaveSlotItem::InitializeSlotItem(FString InSlotId, ESaveSlotState InState)
{
	SlotId = MoveTemp(InSlotId);
	if (ensure(WidgetSwitcher))
	{
		WidgetSwitcher->SetActiveWidgetIndex(static_cast<int32>(InState));
	}
}

void UCCSaveSlotItem::NativeOnClicked()
{
	Super::NativeOnClicked();
	OnSaveSlotSelected.Broadcast(SlotId);
}
