// Fill out your copyright notice in the Description page of Project Settings.


#include "Battle/BattleAnimInstance.h"

#include "BattleUnitActor.h"


// Enemy battle 에 현재 상태 기억용
void UBattleAnimInstance::SetEnemyPhase(EEnemyTurnPhase NewPhase)
{
	EnemyPhase = NewPhase;
	
}

void UBattleAnimInstance::SetPlannedAttack(EEnemyAttackType NewType)
{
	PlannedAttack = NewType; 
	
}

void UBattleAnimInstance::SetTurnState(EBattleTurnState NewState, bool bMyTurn ,bool bPlayerTurnStar)
{
	TurnState = NewState;
	bIsMyTurn = bMyTurn;
	bPlayerTurnStart = bPlayerTurnStar;
	
	if (bPlayerTurnStar)
	{
		bTurnStartRequest = true;
	}
	
	UE_LOG(LogTemp, Warning,
		TEXT("Anim TurnState=%d, bIsMyTurn=%s"),
		(int32)TurnState,
		bIsMyTurn ? TEXT("true") : TEXT("false")
	);
}

void UBattleAnimInstance::RequestAttack()
{
	if (bAttackRequest) return; // 중복 방지

	UE_LOG(LogTemp, Warning, TEXT("[Anim] Attack Requested"));
	
	bAttackRequest = true;
	AttackRequestHoldFrames = 2;	// 프레임 유지용 
	
}

void UBattleAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	
	if (FreeAimShootHoldFrames > 0)
	{
		--FreeAimShootHoldFrames;
		if (FreeAimShootHoldFrames == 0)
		{
			bFreeAimShootRequest = false;
		}
	}
	
	if (AttackRequestHoldFrames > 0)
	{
		--AttackRequestHoldFrames;
		if (AttackRequestHoldFrames == 0)
		{
			bAttackRequest = false; //  두 프레임 지나고 끔
		}
	}

	// TurnStart도 같은 이슈 가능하면 똑같이 처리 추천
	if (bTurnStartRequest)
	{
		bTurnStartRequest = false;
	}
}

void UBattleAnimInstance::OnAttackAnimFinished()
{
	UE_LOG(LogTemp, Warning, TEXT("[Anim] Attack End"));

	AActor* OwnerActor = GetOwningActor();
	
	if (ABattleUnitActor* Unit =
		Cast<ABattleUnitActor>(OwnerActor))
	{
		Unit->OnAttackFinished();
	}
	
}

void UBattleAnimInstance::SetFreeAim(bool bEnable)
{
	bFreeAim = bEnable;
	
	if (!bEnable)
	{
		bFreeAimShootRequest =false;
		FreeAimShootHoldFrames = 0;
		
	}
	
	
}

void UBattleAnimInstance::RequestFreeAimShoot()
{
	if (!bFreeAim) return;                 // 조준 중 아닐 땐 무시
	if (FreeAimShootHoldFrames > 0) return; // 연타 방지(짧게)

	
	UE_LOG(LogTemp, Warning, TEXT("[Anim] FreeAimShoot Requested"));
	
	bFreeAimShootRequest =true;
	FreeAimShootHoldFrames =2;
	
	
}

void UBattleAnimInstance::RequestHit()
{
	bHitRequest = true;
	UE_LOG(LogTemp, Warning, TEXT("[Anim] Hit Requested"));
	
}

void UBattleAnimInstance::ConsumeHit()
{
	bHitRequest = false;
	
}
