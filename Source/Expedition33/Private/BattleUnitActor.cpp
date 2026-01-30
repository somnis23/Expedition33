
#include "BattleUnitActor.h"

#include "BattleGameMode.h"
#include "Components/SkeletalMeshComponent.h"
#include "Battle/BattleAnimInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Battle/BattleTurnManager.h"

//FTimerHandle EnemyAttackTimer;
static const FName TAG_TurnEnd(TEXT("AttackEndPending"));
static const FName TAG_Retreat(TEXT("EnemyRetreatPending"));
ABattleUnitActor::ABattleUnitActor()
{
    
    PrimaryActorTick.bCanEverTick = true
    ;
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
        //  턴 시작 위치를 원위치로 저장
        EnemyHomeLocation = GetActorLocation();

        UE_LOG(LogTemp, Warning, TEXT("[Enemy] TurnStart Home=(%.1f, %.1f, %.1f)"),
            EnemyHomeLocation.X, EnemyHomeLocation.Y, EnemyHomeLocation.Z);

        StartEnemyWalk();
        return;
    }

    // 플레이어는 기존 그대로
    GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ABattleUnitActor::ApplyTurnAnim);
    
}

void ABattleUnitActor::OnTurnEnd()
{
    bIsMyTurn = false;

    UE_LOG(LogTemp, Warning,
        TEXT("[BattleUnitActor] OnTurnEnd"));

    if (UnitType == EBattleUnitType::Enemy)
    {
        SetActorLocation(EnemyHomeLocation);
        SetEnemyPhaseOnAnim(EEnemyTurnPhase::Idle);
    }
    
    
    if (UWorld* World = GetWorld())
    {
        if (ABattleGameMode* GM =
            World->GetAuthGameMode<ABattleGameMode>())
        {
            if (ABattleTurnManager* TM = GM->GetTurnManager())
            {
                TM->EndTurn(this);
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

void ABattleUnitActor::ConfirmAttack()
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

void ABattleUnitActor::OnAttackFinished()
{
    UE_LOG(LogTemp, Warning, TEXT("[Unit] Attack Finished"));

    SetBattleState(EBattleUnitState::Idle);
    OnTurnEnd();
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

void ABattleUnitActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 1) Walk 이동 보간
    if (bIsMyTurn && UnitType == EBattleUnitType::Enemy && bWalking)
    {
        WalkElapsed += DeltaTime;
        const float A = FMath::Clamp(WalkElapsed / EnemyWalkDuration, 0.f, 1.f);

        SetActorLocation(FMath::Lerp(WalkStartLoc, WalkTargetLoc, A));

        if (A >= 1.f)
        {
            bWalking = false;
            StartEnemyCharge();
        }
    }

    // 2) Retreat 이동 보간(원위치 복귀)
    if (bIsMyTurn && UnitType == EBattleUnitType::Enemy && bRetreating)
    {
        RetreatElapsed += DeltaTime;
        const float A = FMath::Clamp(RetreatElapsed / EnemyRetreatDuration, 0.f, 1.f);

        SetActorLocation(FMath::Lerp(RetreatStartLoc, EnemyHomeLocation, A));

        if (A >= 1.f)
        {
            bRetreating = false;
            SetActorLocation(EnemyHomeLocation); // 정확히 스냅
        }
    }

    // 3) 공격 끝 → Retreat 시작(태그)
    if (Tags.Contains(TAG_Retreat))
    {
        Tags.Remove(TAG_Retreat);
        if (bIsMyTurn && UnitType == EBattleUnitType::Enemy)
        {
            StartEnemyRetreat();
        }
    }

    // 4) 점프백 끝 → 턴 종료(태그)
    if (Tags.Contains(TAG_TurnEnd))
    {
        Tags.Remove(TAG_TurnEnd);
        if (bIsMyTurn)
        {
            OnTurnEnd();
        }
    }
}

void ABattleUnitActor::StartEnemyWalk()
{
    SetEnemyPhaseOnAnim(EEnemyTurnPhase::Walk);

    bWalking = true;
    WalkElapsed = 0.f;
    WalkStartLoc = GetActorLocation();

    // “앞으로”는 액터 Forward 기준
    WalkTargetLoc = WalkStartLoc + GetActorForwardVector() * EnemyWalkDistance;
    
}

void ABattleUnitActor::StartEnemyCharge()
{
    SetEnemyPhaseOnAnim(EEnemyTurnPhase::Charge);

    //  Charge 진입 시 Fire/Ice 미리 랜덤 결정
    const EEnemyAttackType Picked = FMath::RandBool() ? EEnemyAttackType::Fire : EEnemyAttackType::Ice;
    SetPlannedAttackOnAnim(Picked);

    UE_LOG(LogTemp, Warning, TEXT("[Enemy] PlannedAttack=%s"),
        (Picked == EEnemyAttackType::Fire) ? TEXT("Fire") : TEXT("Ice"));

    // Charge 지속 후 공격으로
    GetWorld()->GetTimerManager().ClearTimer(EnemyPhaseTimer);
    GetWorld()->GetTimerManager().SetTimer(
        EnemyPhaseTimer, this, &ABattleUnitActor::StartEnemyAttack, EnemyChargeDuration, false);
}

void ABattleUnitActor::StartEnemyAttack()
{
    GetWorld()->GetTimerManager().ClearTimer(EnemyPhaseTimer);

    const bool bFire = (PlannedAttack == EEnemyAttackType::Fire);
    SetEnemyPhaseOnAnim(bFire ? EEnemyTurnPhase::AttackFire : EEnemyTurnPhase::AttackIce);

    UE_LOG(LogTemp, Warning, TEXT("[Enemy] StartAttack=%s"), bFire ? TEXT("Fire") : TEXT("Ice"));

    // 여기서 턴 종료하지 않음
    // 공격 애니 끝(Notify_BattleTag) → EnemyRetreatPending 태그 → StartEnemyRetreat()
}

void ABattleUnitActor::StartEnemyRetreat()
{
    // Retreat(점프백) 애니로 전환
    SetEnemyPhaseOnAnim(EEnemyTurnPhase::Retreat);

    // 원위치로 돌아오는 이동 보간 시작
    bRetreating = true;
    RetreatElapsed = 0.f;
    RetreatStartLoc = GetActorLocation();

    UE_LOG(LogTemp, Warning, TEXT("[Enemy] Retreat Start -> Home"));
    
}

void ABattleUnitActor::SetEnemyPhaseOnAnim(EEnemyTurnPhase Phase)
{
    if (!CharacterMeshComp) return;
    EnemyPhaseRuntime = Phase;
    
    if (UBattleAnimInstance* Anim = Cast<UBattleAnimInstance>(CharacterMeshComp->GetAnimInstance()))
    {
        Anim->SetEnemyPhase(Phase);
    }
    
}

void ABattleUnitActor::SetPlannedAttackOnAnim(EEnemyAttackType Type)
{
    PlannedAttack = Type;

    if (!CharacterMeshComp) return;
    if (UBattleAnimInstance* Anim = Cast<UBattleAnimInstance>(CharacterMeshComp->GetAnimInstance()))
    {
        Anim->SetPlannedAttack(Type);
    }
    
}
