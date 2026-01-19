// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"

#include "EnemyAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	
	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	
	/*UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->MaxWalkSpeed = 300.0f;
	MoveComp->bOrientRotationToMovement = true;
	MoveComp->RotationRate = FRotator(0.f , 600.f ,0.f);
	
	bUseControllerRotationYaw = false;*/
	
}

// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!PlayerPawn)
	{
		PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		return;
	}
	
	float Distance = FVector::Dist(GetActorLocation() , PlayerPawn->GetActorLocation());
	
	AAIController* AICon = Cast<AAIController>(GetController());
	
	if (!AICon) return;
	
	//추적시작
	if (Distance < ChaseRange)
	{
		AICon->MoveToActor(PlayerPawn , 100.f);
		
	}
	// 추적중단 
	else if (Distance > StopRange)
	{
		AICon->StopMovement();
	}
	

}
