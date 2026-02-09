// Fill out your copyright notice in the Description page of Project Settings.


#include "Anim/AnimNotify_BattleTag.h"

#include <ThirdParty/ShaderConductor/ShaderConductor/External/DirectXShaderCompiler/include/dxc/DXIL/DxilConstants.h>

#include "BattleGameMode.h"
#include "BattleUnitActor.h"

void UAnimNotify_BattleTag::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp || TagToAdd.IsNone()) return;
	UE_LOG(LogTemp, Warning, TEXT("[NotifyTag] %s on %s"),
	*TagToAdd.ToString(),
	*GetNameSafe(MeshComp ? MeshComp->GetOwner() : nullptr));
	
	const FString TagStr = TagToAdd.ToString();
	const bool bIsDefTag = 
		TagStr.StartsWith(TEXT("DefStart_")) ||
		TagStr.StartsWith(TEXT("DefOpen_"))  ||
		TagStr.StartsWith(TEXT("DefHit_"))   ||
		TagToAdd == FName(TEXT("DefEnd"));
	
	const bool bIsDodgeTag =
		TagStr.StartsWith("DodgeOpen_") ||
		TagStr.StartsWith("DodgeHit_")  ||
		TagToAdd == FName(TEXT("DodgeEnd"));

	
	if (bIsDefTag || bIsDodgeTag)
	{
		if (UWorld* W = MeshComp->GetWorld())
		{
			if (ABattleGameMode* GM = W->GetAuthGameMode<ABattleGameMode>())
			{
				if (ABattleUnitActor* PlayerUnit = GM->GetBattlePlayerUnit())
				{
					PlayerUnit->PushAnimTag(TagToAdd);
					return;
				}
			}
		}
	}
	
	
	
	
	
	// 1 Mesh Owner에서 바로 찾기
	AActor* Owner = MeshComp->GetOwner();
	for (AActor* Cursor = Owner; Cursor; Cursor = Cursor->GetAttachParentActor())
	{
		if (ABattleUnitActor* Unit = Cast<ABattleUnitActor>(Cursor))
		{
			//Unit->Tags.AddUnique(TagToAdd);
			Unit->PushAnimTag(TagToAdd);
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
				//Unit->Tags.AddUnique(TagToAdd);
				Unit->PushAnimTag(TagToAdd);
				return;
			}
		}
	}
	
	
}
