#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Game/ChessGameTypes.h"
#include "Game/Interactable.h"
#include "ChessPlayer.generated.h"

class AFirstPersonPlayer;
class AChessDesk;
class APlayerController;
class UChessMatchComponent;
class UChessHumanParticipant;
class UCommonInputSubsystem;
class UCameraComponent;
class UInputAction;
class UInteractionWidgetComponent;
class UPositionTweenComponent;
class USceneComponent;
class UStaticMeshComponent;
struct FInputActionValue;
enum class ECommonInputType : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChessChairMoveFinished, bool, bPulledIn);

/* 월드에서 돌아다니는 Pawn이 실제로 체스를 플레이 하기 위해 Possess하는 Pawn */
UCLASS(Blueprintable)
class HIGHSCHOOLCHESSCLUB_API AChessPlayer : public APawn, public IInteractable
{
	GENERATED_BODY()

public:
	AChessPlayer();

	/**
	 * FirstPersonPlayer가 ChessPlayer를 Possess 하기 위해 호출하는 함수
	 * @param InExplorationPawn 월드에 돌아다니는 FirstPersonPlayer
	 */
	UFUNCTION(BlueprintCallable, Category="Chess Player")
	void EnterPlayer(AFirstPersonPlayer* InExplorationPawn);

	/* ChessPlayer를 Unpossess하고 기존의 FirstPersonPlayer로 돌아감 */
	UFUNCTION(BlueprintCallable, Category="Chess Player")
	void ReturnToExploration();

	UFUNCTION(BlueprintPure, Category="ChessPlayer")
	UChessMatchComponent* GetChessMatch() const { return ChessMatch; }

	UFUNCTION(BlueprintPure, Category="Chess Player")
	EChessPlayerPosition GetPlayerPosition() const { return PlayerPosition; }

	UFUNCTION(BlueprintPure, Category="Chess Player")
	bool IsHumanSeat() const { return PlayerPosition == EChessPlayerPosition::PlayerA; }

	UFUNCTION(BlueprintCallable, Category="Chess Player")
	void PullChairIn();

	UFUNCTION(BlueprintCallable, Category="Chess Player")
	void PullChairOut();

	UFUNCTION(BlueprintCallable, Category="Chess Player")
	bool PullChairOutAndDetach(AActor* Occupant);

	UFUNCTION(BlueprintCallable, Category="Chess Player")
	void AttachSeatOccupant(AActor* Occupant);

	UFUNCTION(BlueprintCallable, Category="Chess Player")
	void DetachSeatOccupant(AActor* Occupant);

	UFUNCTION(BlueprintPure, Category="Chess Player")
	bool IsChairPulledIn() const { return bChairPulledIn; }

	UPROPERTY(BlueprintAssignable, Category="Chess Player")
	FOnChessChairMoveFinished OnChairMoveFinished;

	/* ChessMatch 쪽에 실제 게임 시작 요청 */
	UFUNCTION(BlueprintCallable, Category="Chess Player")
	void RequestStartMatch();

#pragma region Interaction
	virtual bool CanInteract_Implementation(APawn* Interactor) override;
	virtual void Interact_Implementation(APawn* Interactor) override;
	virtual FText GetInteractionName_Implementation() override;
#pragma endregion

protected:
#pragma region Actor
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void UnPossessed() override;
#pragma endregion

#pragma region Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chess Player")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chess Player")
	TObjectPtr<UStaticMeshComponent> ChairMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chess Player")
	TObjectPtr<UCameraComponent> ChairCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chess Player")
	TObjectPtr<USceneComponent> SeatAnchor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chess Player")
	TObjectPtr<UPositionTweenComponent> PositionTween;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chess Player")
	TObjectPtr<UInteractionWidgetComponent> InteractionWidget;
#pragma endregion

#pragma region Interaction
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chess Player")
	FText InteractionName;
#pragma endregion

private:
#pragma region Camera
	/* FirstPersonPlayer -> ChessPlayer 시점 전환  */
	void BeginPlayerView(APlayerController* PlayerController);

	/* ChessPlayer -> FirstPersonPlayer 시점 전환  */
	void EndPlayerView(APlayerController* PlayerController);

	/* 매 Tick 카메라 시점 업데이트 */
	void TickCamera(float DeltaTime);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chess Player", meta=(AllowPrivateAccess="true", ClampMin="0.0", ClampMax="180.0"))
	float MaxViewYaw = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chess Player", meta=(AllowPrivateAccess="true", ClampMin="-89.0", ClampMax="89.0"))
	float MinViewPitch = -35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chess Player", meta=(AllowPrivateAccess="true", ClampMin="-89.0", ClampMax="89.0"))
	float MaxViewPitch = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chess Player", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
	float CameraBlendTime = 1.0f;

	FRotator InitialChairCameraRelativeRotation = FRotator::ZeroRotator;
	FRotator InitialViewRotation = FRotator::ZeroRotator;
	float PreviousViewYawMin = 0.0f;
	float PreviousViewYawMax = 0.0f;
	float PreviousViewPitchMin = 0.0f;
	float PreviousViewPitchMax = 0.0f;
	bool bPreviousShowMouseCursor = false;
	bool bLookHold = false;
	bool bStickLookActive = false;
#pragma endregion

#pragma region Runtime State
	UPROPERTY(Transient)
	TObjectPtr<AFirstPersonPlayer> ExplorationPawn;

	UPROPERTY(Transient)
	TObjectPtr<UChessHumanParticipant> HumanParticipant;

	UPROPERTY(Transient)
	TObjectPtr<UCommonInputSubsystem> CommonInputSubsystem;

	UPROPERTY(Transient)
	TObjectPtr<AActor> AttachedSeatOccupant;

	UPROPERTY(Transient)
	TObjectPtr<AActor> SeatOccupantToDetachAfterMove;

	UPROPERTY(Transient)
	TObjectPtr<UChessMatchComponent> ChessMatch;
#pragma endregion

#pragma region Chess
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="ChessPlayer", meta=(AllowPrivateAccess="true"))
	TObjectPtr<AChessDesk> ChessDesk;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Chess Player", meta=(AllowPrivateAccess="true"))
	EChessPlayerPosition PlayerPosition = EChessPlayerPosition::PlayerA;

	UPROPERTY(EditDefaultsOnly, Category="ChessPlayer")
	TSubclassOf<UChessHumanParticipant> ParticipantClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chess Player", meta=(AllowPrivateAccess="true"))
	FVector ChairPulledInLocalOffset = FVector(30.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chess Player", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
	float ChairMoveDuration = 0.35f;

	FVector ChairOutWorldLocation = FVector::ZeroVector;
	bool bChairMovePending = false;
	bool bChairMoveTargetPulledIn = false;
	bool bChairPulledIn = false;

	void MoveChair(bool bPullIn);
	void UpdateChairMove();
#pragma endregion

#pragma region Cursor Movement
	void CursorMoveInput(const FInputActionValue& Value);
	FIntPoint ConvertInputToBoardDelta(FIntPoint InputDelta) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chess Player", meta=(AllowPrivateAccess="true"))
	EChessPlayerPosition InvertedInputPlayerPosition = EChessPlayerPosition::PlayerA;

	/* 현재 마우스 포인터 아래의 체스판 칸을 찾아 HumanParticipant의 논리 커서를 갱신한다. */
	void UpdateCursorFromMouse() const;
#pragma endregion

#pragma region Input Bindings
	void LookInput(const FInputActionValue& Value);
	void LookHoldStarted(const FInputActionValue& InputActionValue);
	void LookHoldEnded(const FInputActionValue& InputActionValue);
	void StickLookStarted(const FInputActionValue& InputActionValue);
	void StickLookInput(const FInputActionValue& InputActionValue);
	void StickLookEnded(const FInputActionValue& InputActionValue);
	void SelectInput(const FInputActionValue& InputActionValue);
	void CancelInput(const FInputActionValue& InputActionValue);

	/**
	 * Common Input에서 사용하는 입력방식이 바뀌었을 때
	 * @param InputType Gamepad 또는 KeyboardMouse
	 */
	void HandleInputMethodChanged(ECommonInputType InputType);
#pragma endregion

#pragma region Input Actions
	UPROPERTY(EditAnywhere, Category="Chess Player")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category="Chess Player")
	TObjectPtr<UInputAction> CursorMoveAction;

	UPROPERTY(EditAnywhere, Category="Chess Player")
	TObjectPtr<UInputAction> LookHoldAction;

	UPROPERTY(EditAnywhere, Category="Chess Player")
	TObjectPtr<UInputAction> JoystickLookAction;

	UPROPERTY(EditAnywhere, Category="Chess Player")
	TObjectPtr<UInputAction> SelectAction;

	UPROPERTY(EditAnywhere, Category="Chess Player")
	TObjectPtr<UInputAction> CancelAction;
#pragma endregion
};
