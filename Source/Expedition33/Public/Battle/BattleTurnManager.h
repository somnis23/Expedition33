// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleTurnManager.generated.h"

class ABattleUnitActor;

UCLASS()
class EXPEDITION33_API ABattleTurnManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABattleTurnManager();
	
	void Initialize(const TArray<ABattleUnitActor*>& InUnits);
	void StartBattle();
	void NextTurn();

private:
	UPROPERTY()
	TArray<ABattleUnitActor*> TurnQueue;
	
	int32 CurrentTurnIndex = INDEX_NONE;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
