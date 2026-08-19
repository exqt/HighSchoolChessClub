#pragma once

#include "CoreMinimal.h"
#include "CCCharacterData.generated.h"

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
};
