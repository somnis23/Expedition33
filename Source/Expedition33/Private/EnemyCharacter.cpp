// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"

#include "EnemyAIController.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundAttenuation.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	
	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	DetectLoopAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("DetectLoopAudio"));
	DetectLoopAudio->SetupAttachment(RootComponent);
	DetectLoopAudio->bAutoActivate = false;
	DetectLoopAudio->bAllowSpatialization = true;
	
	
}

// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
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
		GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;

		AICon->MoveToActor(PlayerPawn, 100.f);
		bIsChasing = true;
		if (DetectLoopAudio && DetectLoopSound)
		{
			if (!DetectLoopAudio->IsPlaying())
			{
				DetectLoopAudio->SetSound(DetectLoopSound);
				DetectLoopAudio->Play();
			}
		}
	}
	// 추적중단 
	else if (Distance > StopRange)
	{
		GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;

		AICon->StopMovement();
		bIsChasing = false;
		if (DetectLoopAudio->IsPlaying())
		{
			DetectLoopAudio->FadeOut(0.5f, 0.f);
		}
	}
}
