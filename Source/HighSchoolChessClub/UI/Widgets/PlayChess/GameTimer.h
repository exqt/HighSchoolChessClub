#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Game/ChessParticipantTypes.h"
#include "GameTimer.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS(Abstract, Blueprintable)
class HIGHSCHOOLCHESSCLUB_API UGameTimer : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="GameTimer")
	void SetInitialTime(int InInitialTime);

	UFUNCTION(BlueprintCallable, Category="GameTimer")
	void SetTime(int InTime) const;

private:
	static FText FormatTime(int InTime);

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ClockText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> ProgressBar;
	
	int InitialTime;
};
