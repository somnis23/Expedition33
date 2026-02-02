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
	
	UPROPERTY(BlueprintReadOnly,Category="Enemy")
	EEnemyTurnPhase EnemyPhase = EEnemyTurnPhase::Idle;
	
	UPROPERTY(BlueprintReadOnly,Category="Enemy")
	EEnemyAttackType PlannedAttack = EEnemyAttackType::Fire;
	
	UFUNCTION(BlueprintCallable , Category= "Enemy")
	void SetEnemyPhase(EEnemyTurnPhase NewPhase);
	
	UFUNCTION(BlueprintCallable, Category="Enemy")
	void SetPlannedAttack(EEnemyAttackType NewType);
	
	UFUNCTION(BlueprintCallable)
	void SetTurnState(EBattleTurnState NewState, bool bMyTurn , bool bPlayerTurnStar);


	UPROPERTY(BlueprintReadOnly,Category= "Battle")
	bool bAttackRequest = false;
	
	UFUNCTION(BlueprintCallable, Category= "Battle")
	void RequestAttack();
	
	
	virtual void NativeUpdateAnimation(float DeltaSeconds)override;

protected:
	UPROPERTY(BlueprintReadOnly)
	bool bTurnStartRequest = false;
	
	int32 AttackRequestHoldFrames = 0;
	
	
public:
	UFUNCTION()
	void OnAttackAnimFinished();
	
	
	//free aim
	UPROPERTY(BlueprintReadOnly,Category="FreeAim")
	bool bFreeAim = false;
	UPROPERTY(BlueprintReadOnly,Category="FreeAim")
	bool bFreeAimShootRequest = false;
	
	UFUNCTION(BlueprintCallable , Category="FreeAim")
	void SetFreeAim(bool bEnable);

	UFUNCTION(BlueprintCallable, Category="FreeAim")
	void RequestFreeAimShoot();
	
	UPROPERTY(BlueprintReadOnly , Category= "Hit")
	bool bHitRequest = false;
	
	UFUNCTION(BlueprintCallable, Category="Hit")
	void RequestHit();
	
	UFUNCTION(BlueprintCallable, Category="Hit")
	void ConsumeHit();
	
	
protected:
	int32 FreeAimShootHoldFrames = 0;
	
};
