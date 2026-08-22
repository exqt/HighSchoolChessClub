#include "UI/Widgets/PlayChess/GameTimer.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UGameTimer::SetInitialTime(const int32 InInitialTime)
{
	InitialTime = FMath::Max(0, InInitialTime);
}

void UGameTimer::SetTime(const int32 InTime) const
{
	ClockText->SetText(FormatTime(InTime));
}

FText UGameTimer::FormatTime(const int InTime)
{
	if (InTime >= 300)
	{
		const int32 TotalSeconds = InTime / 10;
		return FText::FromString(FString::Printf(
			TEXT("%02d:%02d"),
			TotalSeconds / 60,
			TotalSeconds % 60));
	}

	const int32 Seconds = InTime / 10;
	const int32 Hundredths = (InTime % 10) * 10;
	return FText::FromString(FString::Printf(TEXT("%02d.%02d"), Seconds, Hundredths));
}
