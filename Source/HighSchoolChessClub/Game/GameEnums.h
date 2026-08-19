#pragma once

UENUM(BlueprintType)
enum class EChessPlayerColor : uint8
{
	White,
	Black,
	Random
};

UENUM(BlueprintType)
enum class EControlMode : uint8
{
	FirstPerson = 0,
	Chess = 1,
};
