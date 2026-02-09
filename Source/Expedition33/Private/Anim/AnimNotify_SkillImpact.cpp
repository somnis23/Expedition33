// Fill out your copyright notice in the Description page of Project Settings.


#include "Anim/AnimNotify_SkillImpact.h"

#include "BattleUnitActor.h"

void UAnimNotify_SkillImpact::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	
	if (!MeshComp) return;

	ABattleUnitActor* Attacker = Cast<ABattleUnitActor>(MeshComp->GetOwner());
	if (!Attacker) return;

	ABattleUnitActor* Target = Attacker->PendingSkillTarget; // 너가 PendingSkillTarget 저장해둔다고 했지
	if (!Target) Target = Attacker->GetCurrentTarget();
	if (!Target) return;

	const int32 SkillIndex = Attacker->PendingSkillIndex;

	FDamageSpec Spec;
	Spec.Source = EBattleActionType::Skill;

	// 스킬별 데미지
	if (SkillIndex == 0)      Spec.Amount = 25; // Q
	else if (SkillIndex == 1) Spec.Amount = 40; // W
	else                      Spec.Amount = 10;

	Spec.Multiplier = 1.f;

	Target->ApplyDamageSpec(Spec, Attacker);
}
