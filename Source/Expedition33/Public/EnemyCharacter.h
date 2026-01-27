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
	UPROPERTY(EditDefaultsOnly , Category= "Battle")
	bool bCanTriggerBattle= true;
	

	
	UPROPERTY(EditDefaultsOnly, Category="Movement")
	float PatrolSpeed = 200.f;
	UPROPERTY(EditDefaultsOnly, Category="Movement")
	float ChaseSpeed = 400.f;
	
	// 플레이어 추적 사운드용 컴포넌트 생성
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Sound")
	UAudioComponent* DetectLoopAudio;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Sound")
	USoundBase* DetectLoopSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Sound")
	USoundAttenuation* DetectAttenuation;
	

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
	
public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="AI")
	bool bIsChasing;
	
	
	
	APawn* PlayerPawn = nullptr;
	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Battle")
	TSubclassOf<class ABattleEnemyActor> BattleActorClass;
	// 입력값없음
	//virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
