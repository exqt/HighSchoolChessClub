#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CCMapLoadingSubsystem.generated.h"

class UWorld;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCCMapLoadStarted, FName, LevelName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCCMapLoadFinished, UWorld*, LoadedWorld);

UCLASS()
class HIGHSCHOOLCHESSCLUB_API UCCMapLoadingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UCCMapLoadingSubsystem* Get(const UObject* WorldContextObject);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	UFUNCTION(BlueprintCallable, Category = "Map Loading")
	bool LoadTitleLevel();

	UFUNCTION(BlueprintCallable, Category = "Map Loading")
	bool LoadGameLevel();

	UFUNCTION(BlueprintPure, Category = "Map Loading")
	bool IsLoading() const { return bIsLoading; }

	UPROPERTY(BlueprintAssignable, Category = "Map Loading")
	FCCMapLoadStarted OnMapLoadStarted;

	UPROPERTY(BlueprintAssignable, Category = "Map Loading")
	FCCMapLoadFinished OnMapLoadFinished;

private:
	void ShowLoadingScreen() const;
	void HandlePostLoadMap(UWorld* LoadedWorld);

	FDelegateHandle PostLoadMapHandle;
	bool bIsLoading = false;
};
