#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Battle/BattleTypes.h"
#include "Battle/MaelleBattleActor.h"
#include "Battle/BattleEnemyActor.h"
#include "Battle/BattleTurnManager.h"

#include "BattleGameMode.generated.h"

class ABattleUnitActor;
class ABattleCameraActor;

UCLASS()
class EXPEDITION33_API ABattleGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ABattleGameMode();

protected:
	virtual void BeginPlay() override;

protected:
	// === Spawn ===
	void SpawnPlayer();
	void SpawnEnemy();

	// === Spawn Camera ===
	void SpawnBattleCamera();
	
	// === Battle Camera ===
	UPROPERTY(EditDefaultsOnly , Category="Battle Camera")
	TSubclassOf<ABattleCameraActor> BattleCameraClass;
	
	UPROPERTY()
	ABattleCameraActor* BattleCamera;
	
	// === Battle Units ===
	UPROPERTY()
	AMaelleBattleActor* BattlePlayer = nullptr;

	UPROPERTY()
	ABattleEnemyActor* BattleEnemy = nullptr;

	// === Turn Manager ===
	UPROPERTY()
	ABattleTurnManager* TurnManager = nullptr;

protected:
	// === Classes ===
	UPROPERTY(EditDefaultsOnly, Category="Battle")
	TSubclassOf<AMaelleBattleActor> PlayerBattleClass;

	UPROPERTY(EditDefaultsOnly, Category="Battle")
	TSubclassOf<ABattleEnemyActor> EnemyBattleClass;

protected:
	// === Spawn Transforms ===
	FVector PlayerSpawnLocation;
	FRotator PlayerSpawnRotation;

	FVector EnemySpawnLocation;
	FRotator EnemySpawnRotation;
};