// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_SkillEnd.generated.h"

/**
 * 
 */
UCLASS()
class EXPEDITION33_API UAnimNotify_SkillEnd : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	// UAnimNotify에서 override 하는 표준 시그니처
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

	
};
