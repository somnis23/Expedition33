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

	BrodCastTurnState();
	
	TurnQueue[CurrentTurnIndex]->OnTurnStart();
	
}

void ABattleTurnManager::NextTurn()
{
	if (TurnQueue.Num() == 0) return;

	
	CurrentTurnIndex = (CurrentTurnIndex + 1) % TurnQueue.Num();

	UE_LOG(LogTemp, Warning, TEXT("[TurnManager] NextTurn -> %s"),
		*GetNameSafe(TurnQueue[CurrentTurnIndex]));

	BrodCastTurnState();
	
	TurnQueue[CurrentTurnIndex]->OnTurnStart();
	
}

ABattleUnitActor* ABattleTurnManager::GetCurrentUnit() const
{
	if (!TurnQueue.IsValidIndex(CurrentTurnIndex))
		return nullptr;
	
	return TurnQueue[CurrentTurnIndex];
}

void ABattleTurnManager::EndTurn(ABattleUnitActor* Unit)
{
	if (!TurnQueue.IsValidIndex(CurrentTurnIndex))
		return;

	if (TurnQueue[CurrentTurnIndex] != Unit)
		return;

	NextTurn();
	
}

void ABattleTurnManager::BrodCastTurnState()
{
	ABattleUnitActor* Cur = GetCurrentUnit();
	if (!Cur) return;
	
	const EBattleTurnState State =
		Cur->IsEnemyUnit() ? EBattleTurnState::EnemyTurn : EBattleTurnState::PlayerTurn;
	
	for (ABattleUnitActor* Unit : TurnQueue)
	{
		if (!Unit ) continue;
		
		const bool bMyTurn = (Unit == Cur);
		
		Unit->OnTurnStateChanged(State, bMyTurn, false);
		
		
	}
	
	
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

