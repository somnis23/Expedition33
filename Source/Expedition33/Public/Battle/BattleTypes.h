#pragma once

#include "BattleTypes.generated.h"

UENUM(BlueprintType)
enum class EBattleTurnState : uint8
{
	None        UMETA(DisplayName="None"),
	BattleStart UMETA(DisplayName="BattleStart"),
	PlayerTurn  UMETA(DisplayName="PlayerTurn"),
	EnemyTurn   UMETA(DisplayName="EnemyTurn"),
	End         UMETA(DisplayName="End")
};