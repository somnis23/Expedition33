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

//전투 입력 상태 enum
UENUM(BlueprintType)
enum class EBattleCommandState : uint8
{
	None ,
	CommandSelect ,	// 명령 선택 ( 스킬 , 공격 ,등 )
	TargetSelect ,  // 사용할 대상 선택
	Executing		// 실행 중 
};

//몬스터 상태
UENUM(BlueprintType)
enum class EEnemyTurnPhase : uint8
{
	Idle,
	Walk,
	Charge,
	AttackFire,
	AttackIce,
	Retreat
	
};
UENUM(BlueprintType)
enum class EEnemyAttackType : uint8
{
	Fire,
	Ice
	
};
