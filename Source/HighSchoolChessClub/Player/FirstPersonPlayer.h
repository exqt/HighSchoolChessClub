// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FirstPersonPlayer.generated.h"

class UInteractionComponent;
class AChessPlayer;
class AActor;
struct FInputActionValue;
class UInputAction;
class UCameraComponent;

UCLASS()
class HIGHSCHOOLCHESSCLUB_API AFirstPersonPlayer : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UInteractionComponent* InteractionComponent;

protected:
	
#pragma region Input Bindings
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category ="Input")
	TObjectPtr<UInputAction> InteractAction;
#pragma endregion
	
public:
	AFirstPersonPlayer();

	void NotifyChessPlayerEnded(AChessPlayer* EndedPawn);
	
	UFUNCTION(BlueprintCallable, Category="Chess Player")
	bool EnterChessPlayer(AChessPlayer* InChessPlayer);

protected:
	void MoveInput(const FInputActionValue& Value);
	void LookInput(const FInputActionValue& Value);
	void Interact();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAim(float Yaw, float Pitch);

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<AChessPlayer> CurrentChessPlayer;
};
