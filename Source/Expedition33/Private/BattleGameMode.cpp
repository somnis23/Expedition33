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
	//SpawnAndSetBattleCamera();
	//임시 스폰위치
	GetWorldTimerManager().SetTimerForNextTick(this, &ABattleGameMode::SpawnAndSetBattleCamera);
	SetTurnState(EBattleTurnState::BattleStart);
	
	
	
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
	if (!World || !PlayerBattleClass) return;
	
	PlayerSpawnLocation = FVector(0.f , 0.f , 0.f);
	PlayerSpawnRotation = FRotator(0.f , 0.f , 0.f);
	
	AMaelleBattleActor* SpawnedPlayer = World->SpawnActor<AMaelleBattleActor>(
	PlayerBattleClass,
	PlayerSpawnLocation,
	PlayerSpawnRotation
	
	); 
	BattlePlayer = Cast<ABattleUnitActor>(SpawnedPlayer);
	
	if (BattlePlayer)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
		if (PC)
		{
			
			PC->SetViewTargetWithBlend(BattlePlayer, 0.f);
			
			
			UE_LOG(LogTemp, Warning, TEXT("Player Possessed & ViewTarget Set"));
		}
	}
}

void ABattleGameMode::SpawnEnemy()
{
	UWorld* World = GetWorld();
	
	
	UExpeditionGameInstance* GI = GetGameInstance<UExpeditionGameInstance>();
	if (!World || !GI || !GI->BattleEnemyClass) return;
	/*const float Distance = 450.f;  // 대치 거리
	const float Height   = 100.f;*/
	
	EnemySpawnLocation = FVector(400.f , 0.f, 0.f);
	EnemySpawnRotator = FRotator(0.f , 180.f , 0.f);
	
	ABattleEnemyActor* SpawnedEnemy  = World->SpawnActor<ABattleEnemyActor>(
		GI->BattleEnemyClass,
		EnemySpawnLocation,
		EnemySpawnRotator
	);
	
	BattleEnemy = Cast<ABattleUnitActor>(SpawnedEnemy);

	if (!BattleEnemy)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnedEnemy is not BattleUnitActor"));
	}
	
}

void ABattleGameMode::AlignBattleActors()
{
	if (!BattlePlayer || !BattleEnemy)
	{
		UE_LOG(LogTemp, Error, TEXT("AlignBattleActors failed: BattlePlayer or BattleEnemy is null"));
		return;
	}

	const FVector ToPlayer = BattlePlayer->GetActorLocation() - BattleEnemy->GetActorLocation();
	BattleEnemy->SetActorRotation(ToPlayer.Rotation());

	const FVector ToEnemy = BattleEnemy->GetActorLocation() - BattlePlayer->GetActorLocation();
	BattlePlayer->SetActorRotation(ToEnemy.Rotation());
}

void ABattleGameMode::SpawnAndSetBattleCamera()
{
	UWorld* World = GetWorld();
	if (!World || !BattleCameraClass)
	{
		UE_LOG(LogTemp, Error, TEXT("World or BattleCameraClass null"));
		return;
	}

	if (!BattlePlayer || !BattleEnemy)
	{
		UE_LOG(LogTemp, Error, TEXT("Battle actors not ready"));
		return;
	}
	const FVector Center =  (BattlePlayer->GetActorLocation() + BattleEnemy->GetActorLocation()) * 0.5f;
	
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	BattleCamera = World->SpawnActor<ABattleCameraActor>(
		BattleCameraClass,
		Center,
		FRotator::ZeroRotator,
		Params
	);

	if (!BattleCamera)
	{
		UE_LOG(LogTemp, Error, TEXT("BattleCamera spawn failed"));
		return;
	}
	
	BattleCamera->SetupForCenter(Center);
	
	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerController null"));
		return;
	}
	
	PC->SetViewTargetWithBlend(BattleCamera,BattleCamera->BlendTime , VTBlend_Cubic);

}
	
//----------------------턴 시스템 -------------------------
// -------------------------------------------------------
void ABattleGameMode::SetTurnState(EBattleTurnState NewState)
{
	if (CurrentTurnState == NewState)
	{
		return;
	}
	
	CurrentTurnState = NewState;
	
	UE_LOG(LogTemp, Warning, TEXT("Turn State Changed: %d"), (int32)NewState);
	
	OnTurnStateChanged(NewState);
}

void ABattleGameMode::OnTurnStateChanged(EBattleTurnState NewState)
{
	switch (NewState)
	{
	case EBattleTurnState::BattleStart:
		HandleBattleStart();
		break;
		
	case EBattleTurnState::PlayerTurn:
		HandlePlayerTurn();
		break;
		
	case EBattleTurnState::EnemyTurn:
		HandleEnemyTurn();
		break;
		
	case EBattleTurnState::TurnEnd:
		
		break;
		
	case EBattleTurnState::BattleEnd:
		
		break;
		
		
	}
	
	
}

void ABattleGameMode::HandleBattleStart()
{
	UE_LOG(LogTemp, Warning, TEXT("Battle Start"));
    // todo 추후 제거
	
	// 배틀 스타트 연출 후 
	//TODO 임시로 타이머 
	GetWorldTimerManager().SetTimer(
		TurnTimerHandle,
		this,
		&ABattleGameMode::StartPlayerTurn,
		5.0f,
		false
	);
}

void ABattleGameMode::HandlePlayerTurn()
{
	UE_LOG(LogTemp, Warning, TEXT("Player Turn"));

	if (BattlePlayer)
	{
		BattlePlayer->OnTurnStateChanged((int32)EBattleTurnState::PlayerTurn ,
			true);
	}
	
}

void ABattleGameMode::HandleEnemyTurn()
{
	UE_LOG(LogTemp, Warning, TEXT("Enemy Turn"));

	if (BattleEnemy)
	{
		BattleEnemy->OnTurnStart();
	}

	//TODO 임시로 타이머 추후 제거 
	
	GetWorldTimerManager().SetTimer(
		TurnTimerHandle,
		this,
		&ABattleGameMode::EndEnemyTurn,
		1.5f,
		false
	);
	
}

void ABattleGameMode::HandleTurnEnd()
{
}

void ABattleGameMode::StartPlayerTurn()
{
	SetTurnState(EBattleTurnState::PlayerTurn);
	
}

void ABattleGameMode::EndPlayerTurn()
{
	SetTurnState(EBattleTurnState::EnemyTurn);
	
}

void ABattleGameMode::EndEnemyTurn()
{
	
}

