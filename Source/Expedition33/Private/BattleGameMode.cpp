// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleGameMode.h"

#include "ExpeditionGameInstance.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "Kismet/GameplayStatics.h"
#include "Slate/SGameLayerManager.h"

ABattleGameMode::ABattleGameMode()
{
	PrimaryActorTick.bCanEverTick = false;
	DefaultPawnClass = nullptr;
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
	
	SpawnPlayer();
	SpawnEnemy();
	AlignBattleActors();
	//임시 스폰위치
	
	/*
	FVector SpawnLocation(0.f , 0.f, 100.f);
	FRotator SpawnRotation(0.f , 100.f, 0.f);
	
	GetWorld()->SpawnActor<AActor> (
		GI->BattleEnemyClass,
		SpawnLocation,
		SpawnRotation);
		*/
	
	
	
	
}

void ABattleGameMode::SpawnPlayer()
{
	UWorld* World = GetWorld();
	/*BattlePlayer = World->SpawnActor<AMaelleBattleActor>();*/
	
	PlayerSpawnLocation = FVector(0.f , -200.f , 100.f);
	PlayerSpawnRotation = FRotator(0.f , 0.f , 0.f);
	
	AMaelleBattleActor* Player = World->SpawnActor<AMaelleBattleActor>(
	PlayerBattleClass,
	PlayerSpawnLocation,
	PlayerSpawnRotation
	
	); 
	
	
	if (Player)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
		if (PC)
		{
			/*PC->Possess(Player);*/
			PC->SetViewTargetWithBlend(Player, 0.f);
			
			
			UE_LOG(LogTemp, Warning, TEXT("Player Possessed & ViewTarget Set"));
		}
	}
}

void ABattleGameMode::SpawnEnemy()
{
	UWorld* World = GetWorld();
	/*BattleEnemy = World->SpawnActor<ABattleEnemyActor>();*/
	
	UExpeditionGameInstance* GI = GetGameInstance<UExpeditionGameInstance>();
	
	const float Distance = 450.f;  // 대치 거리
	const float Height   = 100.f;
	
	EnemySpawnLocation = FVector(400.f , 0.f, 0.f);
	EnemySpawnRotator = FRotator(0.f , 180.f , 0.f);
	
	ABattleEnemyActor* Enemy = World->SpawnActor<ABattleEnemyActor>(
		GI->BattleEnemyClass,
		EnemySpawnLocation,
		EnemySpawnRotator
	);
	
}

void ABattleGameMode::AlignBattleActors()
{
	if (!BattlePlayer || !BattleEnemy)
	{
		UE_LOG(LogTemp, Error, TEXT("AlignBattleActors failed: null actor"));
		return;
	}

	FVector ToPlayer = BattlePlayer->GetActorLocation() - BattleEnemy->GetActorLocation();
	BattleEnemy->SetActorRotation(ToPlayer.Rotation());

	FVector ToEnemy = BattleEnemy->GetActorLocation() - BattlePlayer->GetActorLocation();
	BattlePlayer->SetActorRotation(ToEnemy.Rotation());
	
	
}
