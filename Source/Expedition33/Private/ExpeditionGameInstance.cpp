// Fill out your copyright notice in the Description page of Project Settings.


#include "ExpeditionGameInstance.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

void UExpeditionGameInstance::EnterBattle(FName InReturnLevel, const FTransform& PlayerTransform, TSubclassOf<AActor> EnemyClass)
{
	Currentphase = EGamePhase::Battle;
	ReturnLevelName = InReturnLevel;
	SavedPlayerTransform = PlayerTransform;
	BattleEnemyClass = EnemyClass;
	bReturnFromBattle = false;
	
}
void UExpeditionGameInstance::ExitBattle()
{
	Currentphase = EGamePhase::World;
	bReturnFromBattle = true;
}

void UExpeditionGameInstance::PlayBGM(USoundBase* NewBGM, float FadeTime)
{
	BGMComponent = UGameplayStatics::SpawnSound2D(
		this,
		NewBGM,
		1.0f,
		1.0f,
		0.f,
		nullptr,
		true   // Persist Across Level Transition
	);

	if (BGMComponent)
	{
		BGMComponent->FadeIn(FadeTime , 1.0f);
		
	}
	
}

void UExpeditionGameInstance::StopBGM(float FadeTime)
{
	if (BGMComponent)
	{
		BGMComponent->FadeOut(FadeTime, 0.f);
		BGMComponent = nullptr;
	}
}
