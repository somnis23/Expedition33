// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

struct FAIStimulus;
/**
 * 
 */
UCLASS()
class EXPEDITION33_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	virtual void OnPossess(APawn* InPawn) override;
	 
	/*UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor , FAIStimulus Stimulus);
	
protected:
	UPROPERTY(VisibleAnywhere)
	UAIPerceptionComponent* Perception;*/
};
