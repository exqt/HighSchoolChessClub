#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Game/ChessParticipantTypes.h"
#include "Game/Interactable.h"
#include "ChessPlayer.generated.h"

class AFirstPersonPlayer;
class AChessMatch;
class APlayerController;
class UChessHumanParticipant;
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
	void SetChessMatch(AChessMatch* InChessMatch) { ChessMatch = InChessMatch; }

	UFUNCTION(BlueprintPure, Category="Chess Player|Chess")
	AChessMatch* GetChessMatch() const { return ChessMatch; }

	virtual bool CanInteract_Implementation(APawn* Interactor) override;
	virtual void Interact_Implementation(APawn* Interactor) override;
	virtual void OnInteractionHover_Implementation(APawn* Interactor) override;
	virtual void OnInteractionUnhover_Implementation(APawn* Interactor) override;
	virtual FText GetInteractionName_Implementation() override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chess Player|Camera", meta=(ClampMin="-89.0", ClampMax="89.0"))
	float MinViewPitch = -35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chess Player|Camera", meta=(ClampMin="-89.0", ClampMax="89.0"))
	float MaxViewPitch = 25.0f;

private:
	void BeginPlayerView(APlayerController* PlayerController);
	void EndPlayerView(APlayerController* PlayerController);
	
#pragma region Input Bindings
	void LookInput(const FInputActionValue& Value);
	void LookHoldStarted(const FInputActionValue& InputActionValue);
	void LookHoldEnded(const FInputActionValue& InputActionValue);
	void TickCamera(float DeltaTime);
	void StickLookStarted(const FInputActionValue& InputActionValue);
	void StickLookInput(const FInputActionValue& InputActionValue);
	void StickLookEnded(const FInputActionValue& InputActionValue);
	void SelectInput(const FInputActionValue& InputActionValue);
	void CancelInput(const FInputActionValue& InputActionValue);
#pragma endregion
	
#pragma region Cursor Movement
	void CursorMoveInput(const FInputActionValue& Value);
	FIntPoint ConvertInputToBoardDelta(FIntPoint InputDelta) const;
#pragma endregion

	UPROPERTY(Transient)
	TObjectPtr<AFirstPersonPlayer> ExplorationPawn;

	UPROPERTY(Transient)
	TObjectPtr<UChessHumanParticipant> HumanParticipant;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Chess Player|Chess", meta=(AllowPrivateAccess="true"))
	TObjectPtr<AChessMatch> ChessMatch;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Chess Player|Chess", meta=(AllowPrivateAccess="true"))
	EChessPlayerPosition PlayerPosition = EChessPlayerPosition::PlayerA;

	TWeakObjectPtr<APlayerController> ViewingController;
	FRotator InitialChairCameraRelativeRotation = FRotator::ZeroRotator;
	FRotator InitialViewRotation = FRotator::ZeroRotator;
	float PreviousViewYawMin = 0.0f;
	float PreviousViewYawMax = 0.0f;
	float PreviousViewPitchMin = 0.0f;
	float PreviousViewPitchMax = 0.0f;

	float CameraBlendTime = 1.0f;
	bool bLookHold = false;
	bool bStickLookActive = false;
	
#pragma region Input Bindings
	UPROPERTY(EditAnywhere, Category ="Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category ="Input")
	TObjectPtr<UInputAction> CursorMoveAction;

	UPROPERTY(EditAnywhere, Category ="Input", meta=(ClampMin="0.0", ClampMax="1.0"))
	float CursorMoveThreshold = 0.5f;
	
	UPROPERTY(EditAnywhere, Category ="Input")
	TObjectPtr<UInputAction> LookHoldAction;
	
	UPROPERTY(EditAnywhere, Category ="Input")
	TObjectPtr<UInputAction> JoystickLookAction;

	UPROPERTY(EditAnywhere, Category ="Input")
	TObjectPtr<UInputAction> SelectAction;

	UPROPERTY(EditAnywhere, Category ="Input")
	TObjectPtr<UInputAction> CancelAction;
#pragma endregion
};
