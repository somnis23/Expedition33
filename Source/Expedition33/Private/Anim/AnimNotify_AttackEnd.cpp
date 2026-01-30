// Fill out your copyright notice in the Description page of Project Settings.


#include "Anim/AnimNotify_AttackEnd.h"

#include "BattleUnitActor.h"
#include "Battle/BattleAnimInstance.h"


void UAnimNotify_AttackEnd::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation)
{
	if (!MeshComp) return;

	//  1) Mesh Owner가 Unit이면 그걸로 끝
	if (ABattleUnitActor* Unit = Cast<ABattleUnitActor>(MeshComp->GetOwner()))
	{
		Unit->Tags.AddUnique(FName("AttackEndPending"));
		return;
	}

	//  2) Pawn 경로 fallback 
	APawn* Pawn = nullptr;
	if (UAnimInstance* AI = MeshComp->GetAnimInstance())
	{
		Pawn = AI->TryGetPawnOwner();
	}
	if (!Pawn) return;

	if (ABattleUnitActor* Unit = Cast<ABattleUnitActor>(Pawn->GetOwner()))
	{
		Unit->Tags.AddUnique(FName("AttackEndPending"));
	}
}
