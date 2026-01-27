// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BattleUnitActor.h"
#include "MaelleBattleActor.h"
#include "MaelleCharacter.h"
#include "Battle/BattleCameraActor.h"
#include "Battle/BattleEnemyActor.h"
#include "GameFramework/GameMode.h"
#include "BattleGameMode.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EBattleTurnState : uint8
{
	None		UMETA(DisplayName="None"),
	BattleStart UMETA(DisplayName="BattleStart"),
	PlayerTurn  UMETA(DisplayName="PlayerTurn"),
	EnemyTurn   UMETA(DisplayName="EnemyTurn"),
	TurnEnd		UMETA(DisplayName="TurnEnd"),
	BattleEnd   UMETA(DisplayName="BattleEnd")
	//EndEnemyTurn	UMETA(DisplayName="EndEnemyTurn")
	
};

UCLASS()
class EXPEDITION33_API ABattleGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	ABattleGameMode();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly , Category="State")
	EBattleTurnState CurrentTurnState = EBattleTurnState::None;
	
	//턴 시스템 
	void SetTurnState(EBattleTurnState NewState);
	void OnTurnStateChanged(EBattleTurnState NewState);
	void HandleBattleStart();
	void HandlePlayerTurn();
	void HandleEnemyTurn();
	void HandleTurnEnd();
	void StartPlayerTurn();
	void EndPlayerTurn();
	void EndEnemyTurn();
	
	FTimerHandle TurnTimerHandle;
protected:
	virtual void BeginPlay() override;
	
private:
	void SpawnPlayer();
	void SpawnEnemy();
	void AlignBattleActors();
	void SpawnAndSetBattleCamera();
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AMaelleBattleActor> PlayerBattleClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ABattleEnemyActor> EnemyBattleClass;
	
	FVector PlayerSpawnLocation;
	FRotator PlayerSpawnRotation;
	
	FVector EnemySpawnLocation;
	FRotator EnemySpawnRotator;
	
	UPROPERTY()
	//AMaelleBattleActor* BattlePlayer = nullptr;
	ABattleUnitActor* BattlePlayer = nullptr;
	UPROPERTY()
	//ABattleEnemyActor* BattleEnemy = nullptr;
	ABattleUnitActor* BattleEnemy = nullptr;
	
	UPROPERTY(EditDefaultsOnly , Category="Battle")
	TSubclassOf<ABattleCameraActor> BattleCameraClass;
	
	UPROPERTY()
	ABattleCameraActor* BattleCamera = nullptr;

};


