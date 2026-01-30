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
UENUM()
enum class EBattleInputMode : uint8
{
	None,
	TargetSelect
};
class ABattleUnitActor;
class ABattleTurnManager;
class ABattleGameMode;


UCLASS()
class EXPEDITION33_API ABattlePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void SetupInputComponent() override;

private:
	EBattleInputMode InputMode = EBattleInputMode::None;

	int32 SelectedEnemyIndex = 0;

	UPROPERTY()
	TArray<TObjectPtr<ABattleUnitActor>> CachedEnemies;

	bool IsPlayerTurn() const;
	ABattleUnitActor* GetCurrentUnit() const;
	void CacheEnemies();

	void ClearEnemySelection();
	void ApplyEnemySelection();

	void OnPressS_AttackMode();
	void OnPressA_PrevTarget();
	void OnPressD_NextTarget();
	void OnPressF_ConfirmTarget();
};
