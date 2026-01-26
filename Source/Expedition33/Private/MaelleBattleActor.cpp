// Fill out your copyright notice in the Description page of Project Settings.


#include "MaelleBattleActor.h"

// Sets default values
AMaelleBattleActor::AMaelleBattleActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMaelleBattleActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMaelleBattleActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMaelleBattleActor::PlayAttack()
{
}

void AMaelleBattleActor::PlayHit()
{
}

