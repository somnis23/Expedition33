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
	
	
	// 카운터 변화 감지 -> 1프레임 펄스 생성
	if (HitCounter != HitCounterSeen)
	{
		bHitEdge = true;
		HitCounterSeen = HitCounter;
	}
	else
	{
		bHitEdge = false;
	}
	
	
	if (FreeAimShootHoldFrames > 0)
	{
		--FreeAimShootHoldFrames;
		if (FreeAimShootHoldFrames == 0)
		{
			bFreeAimShootRequest = false;
		}
	}
	if (ParryRequestHoldFrames > 0)
	{
		--ParryRequestHoldFrames;
		if (ParryRequestHoldFrames == 0)
		{
			bParryRequest = false; // 자동 소멸 
		}
	}
	
	if (PlayerHitHoldFrames > 0)
	{
		--PlayerHitHoldFrames;
		if (PlayerHitHoldFrames == 0)
		{
			bPlayerHit = false; // 자동 소멸
		}
	}
	
	
	if (AttackRequestHoldFrames > 0)
	{
		--AttackRequestHoldFrames;
		if (AttackRequestHoldFrames == 0)
		{
			bAttackRequest = false; //  자동 소멸 
		}
	}
	if (DodgeRequestHoldFrames > 0)
	{
		--DodgeRequestHoldFrames;
		if (DodgeRequestHoldFrames == 0)
		{
			bDodgeRequest = false;
		}
	}
	
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
	HitCounter++;
	UE_LOG(LogTemp, Warning, TEXT("[Anim] RequestHit counter=%d"), HitCounter);
	
}

void UBattleAnimInstance::ConsumeHit()
{
	bHitRequest = false;
	
}

void UBattleAnimInstance::PlayerHit()
{
	if (bPlayerHit) return;
	bPlayerHit =true;
	PlayerHitHoldFrames =2;
}

void UBattleAnimInstance::ConsumePlayerHit()
{
	bPlayerHit = false;
	
}

bool UBattleAnimInstance::RequestParry()
{
	if (bParryPlaying) return false;
	if (bParryRequest) return false;
	bParryRequest = true;
	ParryRequestHoldFrames = 2;
	return true;
}

void UBattleAnimInstance::ConsumeParry()
{
	bParryRequest = false;
}

void UBattleAnimInstance::Notify_ParryStart()
{
	bParryPlaying = true;
	bParryRequest = false; // 진입 즉시 소비(핵심)
	UE_LOG(LogTemp, Warning, TEXT("[Anim] Parry Start"));
}

void UBattleAnimInstance::Notify_ParryEnd()
{
	
	bParryPlaying = false;
	UE_LOG(LogTemp, Warning, TEXT("[Anim] Parry End"));
}

bool UBattleAnimInstance::RequestDodge()
{
	if (bDodgePlaying) return false;
	if (bDodgeRequest) return false;

	bDodgeRequest = true;
	DodgeRequestHoldFrames = 2;
	UE_LOG(LogTemp, Warning, TEXT("[Anim] Dodge Requested"));
	return true;
}


