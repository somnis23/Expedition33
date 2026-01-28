
#include "BattleUnitActor.h"

#include "Components/SkeletalMeshComponent.h"
#include "Battle/BattleAnimInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"

ABattleUnitActor::ABattleUnitActor()
{
    // ⚠️ CDO 안전
    // 생성자에서는 절대 컴포넌트 생성 / 접근 / Find 하지 않는다
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

    UE_LOG(LogTemp, Warning,
        TEXT("[BattleUnitActor] SetBattleState = %d"),
        static_cast<int32>(CurrentState));
}

void ABattleUnitActor::OnTurnStart()
{
    bIsMyTurn = true;
    UE_LOG(LogTemp, Warning, TEXT("[Unit] OnTurnStart CALLED: %s"), *GetName());
    // AnimInstance 생성 타이밍 보장을 위해 한 틱 뒤에 실행
    GetWorld()->GetTimerManager().SetTimerForNextTick(
        this,
        &ABattleUnitActor::ApplyTurnAnim
    );
}

void ABattleUnitActor::OnTurnEnd()
{
    bIsMyTurn = false;

    UE_LOG(LogTemp, Warning,
        TEXT("[BattleUnitActor] OnTurnEnd"));
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