#include "Save/CCSaveGameSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Save/CCSaveGame.h"
#include "Subsystems/SubsystemCollection.h"
#include "World/CCMapLoadingSubsystem.h"

UCCSaveGameSubsystem* UCCSaveGameSubsystem::Get(const UObject* WorldContextObject)
{
	if (!GEngine)
	{
		return nullptr;
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	return World ? UGameInstance::GetSubsystem<UCCSaveGameSubsystem>(World->GetGameInstance()) : nullptr;
}

void UCCSaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<UCCMapLoadingSubsystem>();

	if (UCCMapLoadingSubsystem* MapLoadingSubsystem = GetGameInstance()->GetSubsystem<UCCMapLoadingSubsystem>())
	{
		// MapLoadingSubsystem->OnMapLoadFinished.AddDynamic(this, &ThisClass::HandleMapLoadFinished);
	}
}

void UCCSaveGameSubsystem::Deinitialize()
{
	if (UCCMapLoadingSubsystem* MapLoadingSubsystem = GetGameInstance()->GetSubsystem<UCCMapLoadingSubsystem>())
	{
		MapLoadingSubsystem->OnMapLoadFinished.RemoveAll(this);
	}

	Super::Deinitialize();
}

bool UCCSaveGameSubsystem::SaveCurrentGame()
{
	FString SlotName = CurrentLoadedSlotName;
	UCCSaveGame* SaveGame = Cast<UCCSaveGame>(UGameplayStatics::LoadGameFromSlot( SlotName, 0 ));

	if (const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		SaveGame->LastPosition = PlayerPawn->GetActorLocation();
	}

	return UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, 0);
}

void UCCSaveGameSubsystem::LoadSlotAndTravel(const FString& SlotName)
{
	UCCSaveGame* SaveGame = Cast<UCCSaveGame>(UGameplayStatics::LoadGameFromSlot( SlotName, 0 ));
	CurrentLoadedSlotName = SlotName;
	
	// SaveGame이 없다면 하나 만듦
	if (SaveGame == nullptr)
	{
		SaveGame = Cast<UCCSaveGame>(UGameplayStatics::CreateSaveGameObject(UCCSaveGame::StaticClass()));
		UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, 0);
	}
	
	UCCMapLoadingSubsystem* MapLoadingSubsystem = GetGameInstance()->GetSubsystem<UCCMapLoadingSubsystem>();
	MapLoadingSubsystem->LoadGameLevel();	
}

