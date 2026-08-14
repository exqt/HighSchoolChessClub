#pragma once

#include "CoreMinimal.h"
#include "UI/Core/CCButtonBase.h"
#include "Components/WidgetSwitcher.h"
#include "CCSaveSlotItem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCCSaveSlotItemSelected, FString, SlotId);

UENUM(BlueprintType)
enum class ESaveSlotState : uint8 {
	Empty = 0,
	Occupied = 1,
};

UCLASS(Abstract, Blueprintable)
class HIGHSCHOOLCHESSCLUB_API UCCSaveSlotItem : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Save Slot")
	void InitializeSlotItem(FString InSlotId, ESaveSlotState InState);
	
	UFUNCTION(BlueprintPure, Category = "Save Slot")
	FString GetSlotId() const { return SlotId; }
	
	UPROPERTY(BlueprintAssignable, Category = "Save Slot")
	FCCSaveSlotItemSelected OnSaveSlotSelected;

protected:
	virtual void NativeOnClicked() override;

private:
	UPROPERTY()
	FString SlotId;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher;
};
