
#include "BattleUnitActor.h"

#include "BattleGameMode.h"
#include "Components/SkeletalMeshComponent.h"
#include "Battle/BattleAnimInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Battle/BattleTurnManager.h"

FTimerHandle EnemyAttackTimer;
ABattleUnitActor::ABattleUnitActor()
{
    
    PrimaryActorTick.bCanEverTick = false;
}

void ABattleUnitActor::BeginPlay()
{
    Super::BeginPlay();

    // BP에 존재하는 SkeletalMeshComponent를 런타임에 찾아서 참조
    CharacterMeshComp = FindComponentByClass<USkeletalMeshComponent>();

    UE_LOG(LogTemp, Warning,
        TEXT("[BattleUnitActor] BeginPlay | CharacterMeshComp = %s"),
        *GetNameSafe(CharacterMeshComp));
}

void ABattleUnitActor::SetBattleState(EBattleUnitState NewState)
{
    if (CurrentState == NewState)
    {
        return;
    }

    CurrentState = NewState;

    
        static_cast<int32>(CurrentState);
}

void ABattleUnitActor::OnTurnStart()
{
    bIsMyTurn = true;
    
    if (UnitType == EBattleUnitType::Enemy)
    {
        UE_LOG(LogTemp , Warning , TEXT("Enemy Turn :::"))
        
        GetWorld()->GetTimerManager().SetTimer(EnemyAttackTimer,
            this,
            &ABattleUnitActor::RequestAttack,
            2.5f,
            false);
    }
    else
    {
       
        GetWorld()->GetTimerManager().SetTimerForNextTick(
            this,
            &ABattleUnitActor::ApplyTurnAnim
        );
    }
    
    
    
    
}

void ABattleUnitActor::OnTurnEnd()
{
    bIsMyTurn = false;

    UE_LOG(LogTemp, Warning,
        TEXT("[BattleUnitActor] OnTurnEnd"));

    if (UWorld* World = GetWorld())
    {
        if (ABattleGameMode* GM =
            World->GetAuthGameMode<ABattleGameMode>())
        {
            if (ABattleTurnManager* TM = GM->GetTurnManager())
            {
                TM->NextTurn();
            }
        }
    }
}

void ABattleUnitActor::OnTurnStateChanged(
    EBattleTurnState TurnState,
    bool bMyTurn,
    bool bTurnStart
)
{
    bIsMyTurn = bMyTurn;

    if (!CharacterMeshComp)
    {
        UE_LOG(LogTemp, Error,
            TEXT("[BattleUnitActor] CharacterMeshComp is null"));
        return;
    }

    UBattleAnimInstance* BattleAnim =
        Cast<UBattleAnimInstance>(CharacterMeshComp->GetAnimInstance());

    UE_LOG(LogTemp, Warning,
        TEXT("[BattleUnitActor] OnTurnStateChanged | Anim = %s"),
        *GetNameSafe(BattleAnim));

    if (!BattleAnim)
    {
        UE_LOG(LogTemp, Error,
            TEXT("[BattleUnitActor] Cast to UBattleAnimInstance FAILED"));
        return;
    }

    BattleAnim->SetTurnState(
        TurnState,
        bMyTurn,
        bTurnStart
    );
}

void ABattleUnitActor::EnterAttackMode()
{
    CommandState = EBattleCommandState::TargetSelect ;
    
    UE_LOG(LogTemp, Warning,
        TEXT("[BattleUnitActor] EnterAttackMode → TargetSelect"));
}

void ABattleUnitActor::ConfirmnAttack()
{
    if (!SelectedTarget)    return;
    
    CommandState = EBattleCommandState::Executing;
    
    RequestAttack();
    
}

void ABattleUnitActor::ApplyTurnAnim()
{
    if (!CharacterMeshComp)
    {
        UE_LOG(LogTemp, Error,
            TEXT("[BattleUnitActor] ApplyTurnAnim | CharacterMeshComp is null"));
        return;
    }

    UBattleAnimInstance* BattleAnim =
        Cast<UBattleAnimInstance>(CharacterMeshComp->GetAnimInstance());

    UE_LOG(LogTemp, Warning,
        TEXT("[BattleUnitActor] ApplyTurnAnim | Anim = %s"),
        *GetNameSafe(BattleAnim));
    UE_LOG(LogTemp, Warning, TEXT("[Unit] Cast BattleAnim=%s"), *GetNameSafe(BattleAnim));
    if (!BattleAnim)
    {
        UE_LOG(LogTemp, Error,
            TEXT("[BattleUnitActor] ApplyTurnAnim | Cast FAILED"));
        return;
    }

    // 기본: 플레이어 턴 시작
    BattleAnim->SetTurnState(
        EBattleTurnState::PlayerTurn,
        true,
        true
    );
}

void ABattleUnitActor::RequestAttack()
{
    UBattleAnimInstance* Anim = Cast<UBattleAnimInstance>(CharacterMeshComp->GetAnimInstance());
    UE_LOG(LogTemp, Warning, TEXT("[Unit] RequestAttack Anim=%s"), *GetNameSafe(Anim));

    if (Anim)
    {
        Anim->RequestAttack();
    }
    
}

void ABattleUnitActor::SetSelected(bool bSelected)
{
    if (!CharacterMeshComp) return;

    CharacterMeshComp->SetRenderCustomDepth(bSelected);
    CharacterMeshComp->SetCustomDepthStencilValue(1); // 임시 값

    UE_LOG(LogTemp, Warning,
        TEXT("[Unit] %s Selected = %s"),
        *GetName(),
        bSelected ? TEXT("true") : TEXT("false"));
    
}
