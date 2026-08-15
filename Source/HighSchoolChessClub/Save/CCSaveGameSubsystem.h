#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CCSaveGameSubsystem.generated.h"

class UCCSaveGame;
class USaveGame;
class UWorld;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FCCSaveLoadCompleted, const FString&, SlotName, bool, bSuccess);

UCLASS()
class HIGHSCHOOLCHESSCLUB_API UCCSaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UCCSaveGameSubsystem* Get(const UObject* WorldContextObject);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Save Game")
	bool SaveCurrentGame();

	UFUNCTION(BlueprintCallable, Category = "Save Game")
	void LoadSlotAndTravel(const FString& SlotName);

private:
	FString CurrentLoadedSlotName;
};
