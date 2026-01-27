// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ExpeditionGameInstance.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	World,
	Battle
};



UCLASS()
class EXPEDITION33_API UExpeditionGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	//배틀 진입시 월드 퍼즈
	UPROPERTY(BlueprintReadOnly , Category= "Game")
	EGamePhase Currentphase = EGamePhase::World;
	UPROPERTY(BlueprintReadOnly , Category= "Game")
	FName ReturnLevelName;
	//배틀진입시 월드 에서의 위치
	UPROPERTY(BlueprintReadOnly , Category= "Game")
	FVector SavedWorldTransform;
	
	UPROPERTY(BlueprintReadOnly)
	FTransform SavedPlayerTransform;
	//누구와 전투했는가?
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<class ABattleEnemyActor> BattleEnemyClass;
	
	bool bReturnFromBattle;
	
public:
	UFUNCTION(BlueprintCallable , Category= " Game ")
	void EnterBattle(FName InReturnLevel , const FTransform& PlayerTransform , TSubclassOf<ABattleEnemyActor> EnemyClass);
	UFUNCTION(blueprintCallable, Category= "Game")
	void ExitBattle();
	
	//사운드
	UPROPERTY(EditAnywhere , BlueprintReadOnly , Category="BGM")
	USoundBase* WorldBGM;
	UPROPERTY(EditAnywhere , BlueprintReadOnly , Category="BGM")
	USoundBase* BattleBGM;
	
	void PlayBGM(USoundBase* NewBGM , float FadeTime = 1.0f);
	void StopBGM(float FadeTime = 1.0f);
	
private:
	UPROPERTY()
	UAudioComponent* BGMComponent = nullptr;
};
