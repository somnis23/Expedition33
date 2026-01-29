// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BattleUnitActor.h"
#include "Expedition33/Expedition33Character.h"
#include "GameFramework/PlayerController.h"
#include "BattlePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class EXPEDITION33_API ABattlePlayerController : public APlayerController
{
	GENERATED_BODY()
	
	
protected:
	virtual void SetupInputComponent() override; 
	
	void OnAttackPressed();
	void OnSelectNext();
	void OnSelectPrev();
	void OnConfirm();
	
private:
	UPROPERTY()
	ABattleUnitActor* ControlledUnit = nullptr;
	
	UPROPERTY()
	TArray<ABattleUnitActor*> EnemyUnits;
	
	int32 SelectedIndex = 0;
	bool bSelectingTarget = false;
	
	
public:
	void SetControlledUnit(ABattleUnitActor* Unit);
	void SetEnemyUnits(const TArray<ABattleUnitActor*>& Enemies);
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Input")
	UInputMappingContext* BattleIMC;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Input")
	UInputAction* IA_Attack;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Input")
	UInputAction* IA_Confirm;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Input")
	UInputAction* IA_SelectNext;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Input")
	UInputAction* IA_SelectPrev;
	
};
