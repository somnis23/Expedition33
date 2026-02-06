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
	
	UFUNCTION(BlueprintCallable, Category="Hit")
	void PlayerHit();
	
	UFUNCTION(BlueprintCallable, Category="Hit")
	void ConsumePlayerHit();
	
	//parry
	UPROPERTY(BlueprintReadOnly , Category= "Parry")
	bool bParryRequest = false;
	
	UPROPERTY(BlueprintReadOnly,Category= "Parry")
	bool bParryPlaying = false;
	
	UFUNCTION(BlueprintCallable)
	bool RequestParry();
	
	UFUNCTION(BlueprintCallable)
	void ConsumeParry();
	
	UFUNCTION(BlueprintCallable)
	void Notify_ParryStart();
	
	UFUNCTION(BlueprintCallable)
	void Notify_ParryEnd();
	
	UFUNCTION(BlueprintCallable)
	void Notify_DodgeStart();

	UFUNCTION(BlueprintCallable)
	void Notify_DodgeEnd();
	
	//Hit
	UPROPERTY(BlueprintReadOnly,Category="Hit")
	int32 HitCounter = 0;
	
	UPROPERTY(BlueprintReadOnly , Category="Hit")
	int32 HitCounterSeen = 0;
	
	UPROPERTY(BlueprintReadOnly , Category="Hit")
	bool bHitEdge = false;
	
	UPROPERTY(BlueprintReadWrite , Category= "Hit")
	bool bPlayerHit = false;
	
	//dodge / 회피
	UPROPERTY(BlueprintReadOnly,Category= "Dodge")
	bool bDodgeRequest = false;
	UPROPERTY(BlueprintReadOnly,Category= "Dodge")
	bool bDodgePlaying = false;
	
	UFUNCTION(BlueprintCallable)
	bool RequestDodge();
	
	//반격 counter
	UPROPERTY(BlueprintReadOnly,Category= "Counter")
	bool bCounterRequest = false;
	
	UPROPERTY(BlueprintReadOnly,Category="Counter")
	bool bCounterPlaying = false;	
	
	UFUNCTION(BlueprintCallable, Category= "Counter")
	bool RequestCounter();
	
	UFUNCTION(BlueprintCallable, Category="Counter")
	void Notify_CounterStart();

	UFUNCTION(BlueprintCallable, Category="Counter")
	void Notify_CounterEnd();
	
public:
	UPROPERTY(BlueprintReadOnly, Category="Enemy")
	bool bForceEnemyIdle = false;
	
	UFUNCTION(BlueprintCallable, Category="Enemy")
	void ForceEnemyIdle();
	
protected:
	int32 FreeAimShootHoldFrames = 0;
	
	int32 ParryRequestHoldFrames = 0;
	
	int32 PlayerHitHoldFrames = 0;
	
	int32 DodgeRequestHoldFrames = 0;
	
	int32 CounterRequestHoldFrames = 0;
	
	int32 ForceEnemyIdleHoldFrames = 0;
};
