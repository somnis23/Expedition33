// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BattleGameMode.generated.h"

/**
 * 
 */
UCLASS()
class EXPEDITION33_API ABattleGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	ABattleGameMode();
	
protected:
	virtual void BeginPlay() override;
	
};
