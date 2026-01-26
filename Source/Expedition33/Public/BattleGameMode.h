// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MaelleBattleActor.h"
#include "MaelleCharacter.h"
#include "Battle/BattleEnemyActor.h"
#include "GameFramework/GameMode.h"
#include "BattleGameMode.generated.h"

/**
 * 
 */
UCLASS()
class EXPEDITION33_API ABattleGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	ABattleGameMode();
	
protected:
	virtual void BeginPlay() override;
	
private:
	void SpawnPlayer();
	void SpawnEnemy();
	void AlignBattleActors();
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AMaelleBattleActor> PlayerBattleClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ABattleEnemyActor> EnemyBattleClass;
	
	FVector PlayerSpawnLocation;
	FRotator PlayerSpawnRotation;
	
	FVector EnemySpawnLocation;
	FRotator EnemySpawnRotator;
	
	UPROPERTY()
	AMaelleBattleActor* BattlePlayer = nullptr;
	
	UPROPERTY()
	ABattleEnemyActor* BattleEnemy = nullptr;

};
