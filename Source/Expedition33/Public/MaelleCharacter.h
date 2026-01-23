// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "ExpeditionGameInstance.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "MaelleCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;


UCLASS()
class EXPEDITION33_API AMaelleCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMaelleCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:	
	// 카메라 컴포넌트
	UPROPERTY(VisibleAnywhere , BlueprintReadOnly , Category= "Camera") 
	UCameraComponent* Camera;
	//스프링암 컴포넌트
	UPROPERTY(VisibleAnywhere , BlueprintReadOnly , Category= "Camera")
	USpringArmComponent* SpringArm;
	
protected:
	//INPUT 매핑
	UPROPERTY(EditDefaultsOnly  , Category= "Input")
	UInputMappingContext* DefaultMappingContext;
	
	UPROPERTY(EditDefaultsOnly  , Category= "Input")
	UInputAction* MoveAction;
	
	UPROPERTY(EditDefaultsOnly  , Category= "Input")
	UInputAction* LookAction;
	
	UPROPERTY(EditDefaultsOnly , Category= "Input") 
	UInputAction* JumpAction;
	
	UPROPERTY(EditDefaultsOnly  , Category= "Input")
	UInputAction* SprintAction;
	

protected:
	// anim state용
	UPROPERTY(BlueprintReadOnly  , Category= "Anim")
	float Speed;
	
	UPROPERTY(BlueprintReadOnly  , Category= "Anim")
	bool bIsInAir;
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	
protected:
	void SprintStart();
	void SprintStop();
	
public:
	//스플린트 상태
	UPROPERTY(BlueprintReadOnly , Category= "Movement")
	bool bIsSprint = false;
	//속도
	UPROPERTY(BlueprintReadOnly , Category= "Movement")
	float WalkSpeed = 500.f;
	UPROPERTY(BlueprintReadOnly , Category= "Movement")
	float SprintSpeed  = 900.f;
	
	float SmoothedSpeed = 0.f;
	
	UPROPERTY(BlueprintReadOnly)
	bool bInEnCounter = false;
	
	
	void NotifyActorBeginOverlap(AActor* OtherActor) override;
	
private:
	UExpeditionGameInstance* CachedGameInstance = nullptr;
	
	//전투 진입 요청 
	bool bPendingBattle = false;
	
	// 어느적?
	TSubclassOf<AActor> PendingBattleEnemyClass;
	
	//타이머 핸들
	FTimerHandle BattleTravelTimerHandle;
	
	//타이머로 호출될 함수
	void DoBattleTravel();
	
	bool bEncounterPlaying = false;
	float EncounterDuration = 0.4f;
	float EncounterElapsed = 0.f;
	
	FRotator EncounterStartControlRot;
	FRotator EncounterTargetControlRot;
	
	void StartBattleEncounter(AEnemyCharacter* Enemy);
	void UpdateBattleEncounter(float DeltaTime);
	void FinishBattleEncounter();

	
	
	FTimerHandle BattleEncounterTimer;
};
