// Fill out your copyright notice in the Description page of Project Settings.


#include "Battle/BattleAnimInstance.h"

void UBattleAnimInstance::SetTurnState(int32 InTurnState, bool bMyTurn)
{
	TurnState = InTurnState;
	bIsMyTurn = bMyTurn;
}
