// Fill out your copyright notice in the Description page of Project Settings.


#include "Battle/BattleTurnManager.h"

#include "BattleUnitActor.h"


// Sets default values
ABattleTurnManager::ABattleTurnManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void ABattleTurnManager::Initialize(const TArray<ABattleUnitActor*>& InUnits)
{
	TurnQueue = InUnits;
	CurrentTurnIndex = 0;
	
}

void ABattleTurnManager::StartBattle()
{
	if (TurnQueue.Num() == 0) return;
	
	TurnQueue[CurrentTurnIndex]->OnTurnStart();
	
}

void ABattleTurnManager::NextTurn()
{
	if (TurnQueue.Num() == 0) return;
	TurnQueue[CurrentTurnIndex]->OnTurnEnd();
	
	CurrentTurnIndex = (CurrentTurnIndex + 1) % TurnQueue.Num();
	
	TurnQueue[CurrentTurnIndex]->OnTurnStart();
	
}

// Called when the game starts or when spawned
void ABattleTurnManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABattleTurnManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

