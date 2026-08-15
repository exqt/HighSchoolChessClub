#include "World/CCMapLoadingSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "MoviePlayer.h"
#include "Settings/CCMapLoadingSettings.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"

UCCMapLoadingSubsystem* UCCMapLoadingSubsystem::Get(const UObject* WorldContextObject)
{
	if (!GEngine)
	{
		return nullptr;
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	return World ? UGameInstance::GetSubsystem<UCCMapLoadingSubsystem>(World->GetGameInstance()) : nullptr;
}

void UCCMapLoadingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
		this,
		&ThisClass::HandlePostLoadMap);
}

void UCCMapLoadingSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
	Super::Deinitialize();
}

bool UCCMapLoadingSubsystem::LoadTitleLevel()
{
	const UCCMapLoadingSettings* Settings = GetDefault<UCCMapLoadingSettings>();
	UGameplayStatics::OpenLevelBySoftObjectPtr(GetGameInstance(), Settings->TitleLevel);
	return true;
}

bool UCCMapLoadingSubsystem::LoadGameLevel()
{
	const UCCMapLoadingSettings* Settings = GetDefault<UCCMapLoadingSettings>();
	UGameplayStatics::OpenLevelBySoftObjectPtr(GetGameInstance(), Settings->GameLevel);
	return true;
}

void UCCMapLoadingSubsystem::ShowLoadingScreen() const
{
}

void UCCMapLoadingSubsystem::HandlePostLoadMap(UWorld* LoadedWorld)
{
}
