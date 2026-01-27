// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Battle/BattleAnimInstance.h"
#include "GameFramework/Actor.h"
#include "BattleUnitActor.generated.h"
UENUM(BlueprintType)
enum class EBattleUnitState : uint8
{
	Idle,
	Attack,
	Hit,
	Dead
};


UCLASS()
class EXPEDITION33_API ABattleUnitActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABattleUnitActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	
	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* Mesh;

	UPROPERTY(BlueprintReadOnly)
	EBattleUnitState CurrentState = EBattleUnitState::Idle;
	
	UPROPERTY(BlueprintReadOnly)
	UBattleAnimInstance* BattleAnimInstance;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	UFUNCTION(BlueprintCallable)
	void SetBattleState(EBattleUnitState NewState);

	FORCEINLINE USkeletalMeshComponent* GetMesh() const { return Mesh; }
	
	UFUNCTION(BlueprintCallable)
	virtual void OnTurnStart();
	
	UFUNCTION(BlueprintCallable)
	void OnTurnStateChanged(int32 TurnState , bool bMyTurn);
	
	
	UFUNCTION(BlueprintCallable)
	virtual void OnTurnEnd();
};
