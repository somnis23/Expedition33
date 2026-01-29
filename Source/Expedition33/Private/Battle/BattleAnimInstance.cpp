// Fill out your copyright notice in the Description page of Project Settings.


#include "Battle/BattleAnimInstance.h"

#include "BattleUnitActor.h"

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
	
}

void UBattleAnimInstance::Animnotify_AttackEnd()
{
	UE_LOG(LogTemp , Error , TEXT("Anim ::::: >> Attack End "));
	
	if (ABattleUnitActor* Unit = Cast<ABattleUnitActor>(TryGetPawnOwner()))
	{
		Unit->OnTurnEnd();
	}
	
}
