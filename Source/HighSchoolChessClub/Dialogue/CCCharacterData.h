#pragma once

#include "CoreMinimal.h"
#include "CCCharacterData.generated.h"

USTRUCT(BlueprintType)
struct FCCSmallTalkDialogue
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CC Small Talk Dialogue")
	TArray<FText> Lines;
};

UCLASS(BlueprintType)
class HIGHSCHOOLCHESSCLUB_API UCCCharacterData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("CharacterData"), GetFName());
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character")
	FText CharacterName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character")
	FText Association;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character")
	TArray<FCCSmallTalkDialogue> SmallTalkDialogues;
};
