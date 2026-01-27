// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleUnitActor.h"

// Sets default values
ABattleUnitActor::ABattleUnitActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

// Called when the game starts or when spawned
void ABattleUnitActor::BeginPlay()
{
	Super::BeginPlay();
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		BattleAnimInstance = Cast<UBattleAnimInstance>(MeshComp->GetAnimInstance());
	}
	
}

// Called every frame
void ABattleUnitActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABattleUnitActor::SetBattleState(EBattleUnitState NewState)
{
	CurrentState = NewState;
	
}

void ABattleUnitActor::OnTurnStart()
{
		
	//TODO UI 활성화 한 후 각 함수로 분기 
}

void ABattleUnitActor::OnTurnStateChanged(int32 TurnState, bool bMyTurn)
{
	if (BattleAnimInstance)
	{
		BattleAnimInstance->SetTurnState(TurnState,bMyTurn);
	}
	
}

void ABattleUnitActor::OnTurnEnd()
{
	
	UE_LOG(LogTemp, Warning, TEXT("BattleUnitActor OnTurnEnd"));
}

