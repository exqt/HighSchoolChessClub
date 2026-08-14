#include "UI/Widgets/SaveSlots/CCSaveSlotScreen.h"

#include "Components/DynamicEntryBox.h"
#include "Kismet/GameplayStatics.h"
#include "Save/CCSaveGame.h"
#include "Save/CCSaveGameSubsystem.h"
#include "UI/Widgets/SaveSlots/CCSaveSlotItem.h"

void UCCSaveSlotScreen::NativeOnActivated()
{
	Super::NativeOnActivated();
	RebuildSaveSlots();
}

void UCCSaveSlotScreen::RebuildSaveSlots()
{
	if (!DynamicEntryBox) return;

	DynamicEntryBox->Reset<UCCSaveSlotItem>(
		[this](UCCSaveSlotItem& Item) {
			Item.OnSaveSlotSelected.RemoveAll(this);
		}
	);

	DesiredFocusItem = nullptr;
	
	for (int i = 0; i < Max_Slots; i++)
	{
		UCCSaveSlotItem* ItemWidget = DynamicEntryBox->CreateEntry<UCCSaveSlotItem>();
		if (!ensureMsgf(ItemWidget, TEXT("DynamicEntryBox Entry Widget Class must inherit from UCCSaveSlotItem.")))
		{
			continue;
		}

		FString SlotName = FString::Printf(TEXT("SaveSlot%d"), i);
	
		if (UCCSaveGame* LoadedGame = Cast<UCCSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0)))
		{
			ItemWidget->InitializeSlotItem(SlotName, ESaveSlotState::Occupied);
			if (!DesiredFocusItem)
			{
				DesiredFocusItem = ItemWidget;
			}
		}
		else
		{
			ItemWidget->InitializeSlotItem(SlotName, ESaveSlotState::Empty);
		}

		ItemWidget->OnSaveSlotSelected.AddUniqueDynamic(this, &ThisClass::HandleSaveSlotSelected);
	}

	RequestRefreshFocus();
}

UWidget* UCCSaveSlotScreen::NativeGetDesiredFocusTarget() const
{
	return DesiredFocusItem ? DesiredFocusItem.Get() : Super::NativeGetDesiredFocusTarget();
}

void UCCSaveSlotScreen::HandleSaveSlotSelected(FString SlotId)
{
	if (UCCSaveGameSubsystem* SaveGameSubsystem = UCCSaveGameSubsystem::Get(this))
	{
		SaveGameSubsystem->LoadSlotAndTravel(SlotId);
	}
}
