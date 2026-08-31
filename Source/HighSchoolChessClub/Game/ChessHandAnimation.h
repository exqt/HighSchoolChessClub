#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ChessHandAnimation.generated.h"

struct FChessMoveAnimationData;

UINTERFACE(BlueprintType, meta=(CannotImplementInterfaceInBlueprint))
class HIGHSCHOOLCHESSCLUB_API UChessHandAnimation : public UInterface
{
	GENERATED_BODY()
};

class HIGHSCHOOLCHESSCLUB_API IChessHandAnimation
{
	GENERATED_BODY()

public:
	virtual void StartChessHandAnimation(const FChessMoveAnimationData& MoveData) = 0;
};
