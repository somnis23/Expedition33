// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MaelleBattleActor.generated.h"

UCLASS()
class EXPEDITION33_API AMaelleBattleActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMaelleBattleActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
public:
	UPROPERTY(VisibleAnywhere)
	USkeletalMesh* Mesh ;
	
	UPROPERTY(EditAnywhere)
	UAnimInstance* AnimInstance;
	
	UPROPERTY(BlueprintReadOnly)
	bool bIsMyTurn = false;
	
	UFUNCTION(BlueprintCallable)
	void PlayAttack();
	
	UFUNCTION(BlueprintCallable)
	void PlayHit();
	
};
