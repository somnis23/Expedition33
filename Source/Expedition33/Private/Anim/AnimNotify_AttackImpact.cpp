// Fill out your copyright notice in the Description page of Project Settings.


#include "Anim/AnimNotify_AttackImpact.h"

#include "BattleUnitActor.h"

void UAnimNotify_AttackImpact::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	
	if (!MeshComp) return;

	ABattleUnitActor* Attacker = Cast<ABattleUnitActor>(MeshComp->GetOwner());
	if (!Attacker) return;

	ABattleUnitActor* Target = Attacker->GetCurrentTarget(); 
	if (!Target) return;

	FDamageSpec Spec;
	Spec.Source = EBattleActionType::Attack;
	Spec.Amount = 15;      // 공격력 // 스탯기반
	Spec.Multiplier = 1.f;

	Target->ApplyDamageSpec(Spec, Attacker);
}
