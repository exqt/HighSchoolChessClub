#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Game/Interactable.h"
#include "ChessPlayer.generated.h"

class AFirstPersonPlayer;
class AChessDesk;
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

	UFUNCTION(BlueprintCallable, Category="Chess Player|Chess")
	void SetChessDesk(AChessDesk* InChessDesk) { ChessDesk = InChessDesk; }

	UFUNCTION(BlueprintPure, Category="Chess Player|Chess")
	AChessDesk* GetChessDesk() const { return ChessDesk; }

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chess Player|Camera", meta=(ClampMin="0.0", ClampMax="180.0"))
	float MaxViewYaw = 60.0f;

	/** Minimum pitch offset from the chair's initial view. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chess Player|Camera", meta=(ClampMin="-89.0", ClampMax="89.0"))
	float MinViewPitch = -35.0f;

	/** Maximum pitch offset from the chair's initial view. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chess Player|Camera", meta=(ClampMin="-89.0", ClampMax="89.0"))
	float MaxViewPitch = 25.0f;

private:
	void BeginPlayerView(APlayerController* PlayerController);
	void EndPlayerView(APlayerController* PlayerController);
	void LookInput(const FInputActionValue& Value);
	void CursorMoveInput(const FInputActionValue& Value);

	UPROPERTY(Transient)
	TObjectPtr<AFirstPersonPlayer> ExplorationPawn;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Chess Player|Chess", meta=(AllowPrivateAccess="true"))
	TObjectPtr<AChessDesk> ChessDesk;

	TWeakObjectPtr<APlayerController> ViewingController;
	FRotator InitialChairCameraRelativeRotation = FRotator::ZeroRotator;
	float PreviousViewYawMin = 0.0f;
	float PreviousViewYawMax = 0.0f;
	float PreviousViewPitchMin = 0.0f;
	float PreviousViewPitchMax = 0.0f;
	
	float CameraBlendTime = 1.0f;
	
#pragma region Input Bindings
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* LookAction;

	/** A 2D axis action. IA_Move can be reused here. */
	UPROPERTY(EditAnywhere, Category ="Input")
	TObjectPtr<UInputAction> CursorMoveAction;

	/** Each stick axis must reach this value before it becomes -1 or 1. */
	UPROPERTY(EditAnywhere, Category ="Input", meta=(ClampMin="0.0", ClampMax="1.0"))
	float CursorMoveThreshold = 0.5f;
#pragma endregion
};
