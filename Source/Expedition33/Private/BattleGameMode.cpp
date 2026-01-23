// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleGameMode.h"

#include "ExpeditionGameInstance.h"

ABattleGameMode::ABattleGameMode()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABattleGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	UExpeditionGameInstance* GI = GetGameInstance<UExpeditionGameInstance>();
	
	if (!GI)
	{
		UE_LOG(LogTemp, Warning, TEXT("Game Instance : is Null"));
		return;
	}
	
	if (!GI->BattleEnemyClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("BattleEnemyClass : is Null"));
		
		return;
	}
	//임시 스폰위치
	
	FVector SpawnLocation(0.f , 0.f, 100.f);
	FRotator SpawnRotation(0.f , 100.f, 0.f);
	
	GetWorld()->SpawnActor<AActor> (
		GI->BattleEnemyClass,
		SpawnLocation,
		SpawnRotation);
	
	
	
	
}
