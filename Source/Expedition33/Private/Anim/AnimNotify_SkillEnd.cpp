// Fill out your copyright notice in the Description page of Project Settings.


#include "Anim/AnimNotify_SkillEnd.h"

#include "BattleUnitActor.h"
#include "Battle/BattleAnimInstance.h"

void UAnimNotify_SkillEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	
	if (!MeshComp) return;

	ABattleUnitActor* Unit = Cast<ABattleUnitActor>(MeshComp->GetOwner());
	if (!Unit) return;

	// 1) 애님 상태 원복(Idle)
	if (UBattleAnimInstance* Anim = Cast<UBattleAnimInstance>(MeshComp->GetAnimInstance()))
	{
		Anim->Action = EBattleUnitState::Idle;
		Anim->bSkillRequested = false;
		//Anim->SkillIndex = INDEX_NONE;
	}

	// 2) 턴 종료
	Unit->OnTurnEnd();
}
