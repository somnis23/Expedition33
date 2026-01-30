// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_BattleTag.generated.h"

/**
 * 
 */
UCLASS()
class EXPEDITION33_API UAnimNotify_BattleTag : public UAnimNotify
{
	GENERATED_BODY()
	
	// 에디터에서 태그 선택
	UPROPERTY(EditAnywhere, Category="Battle")
	FName TagToAdd = NAME_None;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
