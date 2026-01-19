// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

UCLASS()
class EXPEDITION33_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(EditAnywhere , Category= "AI")
	float ChaseRange = 1000.f;
	
	UPROPERTY(EditAnywhere, Category= "AI")
	float StopRange = 1200.f;
	
	APawn* PlayerPawn = nullptr;
	
	// 입력값없음
	//virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
