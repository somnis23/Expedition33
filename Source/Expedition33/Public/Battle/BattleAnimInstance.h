// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Battle/BattleTypes.h"
#include "BattleAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class EXPEDITION33_API UBattleAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere,BlueprintReadOnly , Category="Battle")
	EBattleTurnState TurnState;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly , Category="Battle")
	bool bIsMyTurn = false;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly , Category="Battle")
	bool bPlayerTurnStart =false;
	
	UPROPERTY(BlueprintReadOnly)
	bool bCanAct = false;
	
	
	
	
	UFUNCTION(BlueprintCallable)
	void SetTurnState(EBattleTurnState NewState, bool bMyTurn , bool bPlayerTurnStar);


	UPROPERTY(BlueprintReadOnly,Category= "Battle")
	bool bAttackRequest = false;
	
	UFUNCTION(BlueprintCallable, Category= "Battle")
	void RequestAttack();

protected:
	UPROPERTY(BlueprintReadOnly)
	bool bTurnStartRequest = false;
	
	
public:
	UFUNCTION()
	void Animnotify_AttackEnd();
	
};
