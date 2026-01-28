// Fill out your copyright notice in the Description page of Project Settings.


#include "Battle/BattleAnimInstance.h"

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
