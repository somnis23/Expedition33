// Fill out your copyright notice in the Description page of Project Settings.


#include "Anim/AnimNotify_BattleTag.h"

#include "BattleUnitActor.h"

void UAnimNotify_BattleTag::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp || TagToAdd.IsNone()) return;

	// 1 Mesh Owner에서 바로 찾기
	AActor* Owner = MeshComp->GetOwner();
	for (AActor* Cursor = Owner; Cursor; Cursor = Cursor->GetAttachParentActor())
	{
		if (ABattleUnitActor* Unit = Cast<ABattleUnitActor>(Cursor))
		{
			UE_LOG(LogTemp , Warning ,TEXT("ANIMnotify  :: mesh owner"));
			Unit->Tags.AddUnique(TagToAdd);
			return;
		}
	}

	// 2 혹시 Pawn 경로인 경우도 대비
	if (UAnimInstance* AI = MeshComp->GetAnimInstance())
	{
		if (APawn* Pawn = AI->TryGetPawnOwner())
		{
			if (ABattleUnitActor* Unit = Cast<ABattleUnitActor>(Pawn->GetOwner()))
			{
				UE_LOG(LogTemp , Warning ,TEXT("ANIMnotify  :: mesh pawn"));
				Unit->Tags.AddUnique(TagToAdd);
				return;
			}
		}
	}
	
	
}
