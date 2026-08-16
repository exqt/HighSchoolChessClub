#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Game/Interactable.h"
#include "ChessPlayer.generated.h"

class AFirstPersonPlayer;
class APlayerController;
class UCameraComponent;
class UInputAction;
class USceneComponent;
class UStaticMeshComponent;
struct FInputActionValue;
struct FMinimalViewInfo;

UENUM(BlueprintType)
enum class EChessPlayerMode : uint8
{
	Seated,
	PlayingChess
};

/** A possessable chair shared by the seated and chess gameplay modes. */
UCLASS(Blueprintable)
class HIGHSCHOOLCHESSCLUB_API AChessPlayer : public APawn, public IInteractable
{
	GENERATED_BODY()

public:
	AChessPlayer();

	UFUNCTION(BlueprintCallable, Category="Chess Player")
	bool EnterPlayer(AFirstPersonPlayer* InExplorationPawn);

	UFUNCTION(BlueprintCallable, Category="Chess Player")
	void ReturnToExploration();

	virtual bool CanInteract_Implementation(APawn* Interactor) override;
	virtual void Interact_Implementation(APawn* Interactor) override;
	virtual void OnInteractionHover_Implementation(APawn* Interactor) override;
	virtual void OnInteractionUnhover_Implementation(APawn* Interactor) override;
	virtual FText GetInteractionName_Implementation() override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
#pragma region Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chess Player")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chess Player")
	TObjectPtr<UStaticMeshComponent> ChairMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chess Player")
	TObjectPtr<UCameraComponent> ChairCamera;
#pragma endregion

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	FText InteractionName;

private:
	void BeginPlayerView(APlayerController* PlayerController);
	void EndPlayerView(APlayerController* PlayerController);
	void LookInput(const FInputActionValue& Value);

	UPROPERTY(Transient)
	TObjectPtr<AFirstPersonPlayer> ExplorationPawn;

	TWeakObjectPtr<APlayerController> ViewingController;
	FRotator InitialChairCameraRelativeRotation = FRotator::ZeroRotator;
	
	float CameraBlendTime = 1.0f;
	
#pragma region Input Bindings
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* LookAction;
#pragma endregion
};
