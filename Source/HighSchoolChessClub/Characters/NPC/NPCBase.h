#pragma once

#include "CoreMinimal.h"
#include "Game/Interactable.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "Dialogue/CCCharacterData.h"
#include "Game/ChessGameTypes.h"
#include "Game/ChessHandAnimation.h"
#include "NPCBase.generated.h"

class UInteractionWidgetComponent;
class UChessBotParticipant;
class UChessParticipant;

UENUM(BlueprintType)
enum class ENPCState : uint8
{
	StandingIdle,
	Talking,
	Texting,
	Walking,
	Playing
};

UCLASS(Blueprintable)
class HIGHSCHOOLCHESSCLUB_API ANPCBase : public ACharacter, public IInteractable, public IChessHandAnimation
{
	GENERATED_BODY()

public:
	ANPCBase();

	virtual bool CanInteract_Implementation(APawn* Interactor) override;
	virtual FText GetInteractionName_Implementation() override;

	UFUNCTION(BlueprintCallable, Category="NPC Base", meta=(DisplayName="Set NPC State"))
	void SetNPCState(ENPCState InNPCState) { NPCState = InNPCState; }

	UFUNCTION(BlueprintPure, Category="NPC Base", meta=(DisplayName="Get NPC State"))
	ENPCState GetNPCState() const { return NPCState; }
	
	UFUNCTION(BlueprintImplementableEvent, Category="NPC Base")
	void StartChessMoveAnimation(const FChessMoveAnimationData& MoveData);

	UFUNCTION(BlueprintCallable, Category="NPC Base")
	void FinishChessMoveAnimation(int32 MoveId);

	virtual void StartChessHandAnimation(const FChessMoveAnimationData& MoveData) override { StartChessMoveAnimation(MoveData); }

	UFUNCTION(BlueprintPure, Category="NPCBase")
	UChessParticipant* GetChessParticipant() const { return ActiveChessParticipant; }

	UFUNCTION(BlueprintPure, Category="NPCBase")
	TSubclassOf<UChessBotParticipant> GetChessParticipantClass() const { return ChessParticipantClass; }

	void SetChessParticipant(UChessParticipant* InChessParticipant) { ActiveChessParticipant = InChessParticipant; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="NPC Base")
	TObjectPtr<UInteractionWidgetComponent> InteractionWidget;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	TSoftObjectPtr<UCCCharacterData> CharacterData;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Dialogue")
	bool bIsDialogueActive = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="NPC Base")
	ENPCState NPCState = ENPCState::StandingIdle;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category="NPCBase")
	TObjectPtr<UChessParticipant> ActiveChessParticipant;

	UPROPERTY(EditDefaultsOnly, Category="NPCBase")
	TSubclassOf<UChessBotParticipant> ChessParticipantClass;

private:
	UFUNCTION()
	void HandleDialogueActiveChanged(bool bIsActive);
};
