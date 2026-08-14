#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "CCSaveSlotScreen.generated.h"

class UCCSaveSlotItem;
class UDynamicEntryBox;

UCLASS(Abstract, Blueprintable)
class HIGHSCHOOLCHESSCLUB_API UCCSaveSlotScreen : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	const int Max_Slots = 5;	
	
	UFUNCTION(BlueprintCallable, Category = "Save Slot")
	void RebuildSaveSlots();

protected:
	virtual void NativeOnActivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

private:
	UFUNCTION()
	void HandleSaveSlotSelected(FString SlotId);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox> DynamicEntryBox;

	UPROPERTY(Transient)
	TObjectPtr<UCCSaveSlotItem> DesiredFocusItem;
};
