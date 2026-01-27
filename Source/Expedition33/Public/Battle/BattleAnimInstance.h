// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BattleAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class EXPEDITION33_API UBattleAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly , Category="Battle")
	int32 TurnState = 0;
	
	UPROPERTY(BlueprintReadOnly , Category="Battle")
	bool bIsMyTurn = false;
	
	void SetTurnState(int32 InTurnState , bool bMyTurn);
	
};
