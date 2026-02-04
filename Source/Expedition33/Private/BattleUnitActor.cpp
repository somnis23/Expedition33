
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
static const FName TAG_HitConsume(TEXT("HITConsumePending"));
static const FString PFX_Start(TEXT("DefStart_"));
static const FString PFX_Open(TEXT("DefOpen_"));
static const FString PFX_Hit(TEXT("DefHit_"));
static const FName   TAG_End(TEXT("DefEnd"));

static const FName TAG_ParryStart(TEXT("ParryStart"));
static const FName TAG_ParryEnd(TEXT("ParryEnd"));

static const FString TAG_DodgeOpen(TEXT("DodgeOpen_"));
static const FString TAG_DodgeHit(TEXT("DodgeHit_"));
static const FName TAG_DodgeEnd(TEXT("DodgeEnd"));

// 숫자 파싱 
static bool ParseIndex(const FString& Str, const FString& Prefix, int32& OutIndex)
{
    if (!Str.StartsWith(Prefix)) return false;
    const FString NumStr = Str.Mid(Prefix.Len());
    if (NumStr.IsEmpty()) return false;
    OutIndex = FCString::Atoi(*NumStr);
    return true;
}



ABattleUnitActor::ABattleUnitActor()
{
    
    PrimaryActorTick.bCanEverTick = true;
    CacheCharacterMesh();
    UE_LOG(LogTemp, Warning, TEXT("[Unit] BeginPlay %s Mesh=%s"),
        *GetName(), *GetNameSafe(CharacterMeshComp));
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
}


void ABattleUnitActor::OnTurnStart()
{
    bIsMyTurn = true;
    ResetPatternState();
    bParryWindowOpen = false;
    if (UnitType != EBattleUnitType::Enemy)
    {
        
        if (CharacterMeshComp)
        {
            if (UBattleAnimInstance* Anim = Cast<UBattleAnimInstance>(CharacterMeshComp->GetAnimInstance()))
            {
                Anim->ConsumePlayerHit(); // 피격 잔상 제거
                Anim->SetFreeAim(false);  // FreeAim 잔상 제거
            }
        }
        
        RefillCostForTurn();   // 플레이어 턴 시작 시 리필
        GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ABattleUnitActor::ApplyTurnAnim);
       
        return;
    }
    
    
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
    ConsumeAnimTags();
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
            if (bIsMyTurn && UnitType == EBattleUnitType::Enemy)
            {
                OnTurnEnd();
            }
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
    
    if (Tags.Contains(TAG_HitConsume))
    {
        Tags.Remove(TAG_HitConsume);
        
        if (CharacterMeshComp)
        {
            if (UBattleAnimInstance* Anim = Cast<UBattleAnimInstance>(CharacterMeshComp->GetAnimInstance()))
            {
                UE_LOG(LogTemp, Warning, TEXT("[Unit] ConsumeHit by Tag"));
                Anim->ConsumeHit();   // bHitRequest = false
                
            }
        }
    }
    
    if (bDodgeIntent && GetWorld()->TimeSeconds > DodgeIntentUntilTime)
    {
        bDodgeIntent = false;
    }
    
    
}

bool ABattleUnitActor::SpendCost(int32 Amount)
{
    
    if (!CanSpendCost(Amount)) return false;

    Cost -= Amount;
    UE_LOG(LogTemp, Warning, TEXT("[Unit] SpendCost %d -> Cost=%d"), Amount, Cost);

    // 코스트 0이면 턴 종료 (원작식)
    if (Cost <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Unit] Cost empty -> EndTurn"));
        OnTurnEnd();
    }
    return true;
}

void ABattleUnitActor::OnFreeAimHit(const FName HitBone)
{
    if (!CharacterMeshComp) return;
    
    UE_LOG(LogTemp, Warning, TEXT("[Unit] OnFreeAimHit %s bone=%s"),
        *GetName(), *HitBone.ToString());

    USkeletalMeshComponent* Mesh = GetCharacterMesh();
    UE_LOG(LogTemp, Warning, TEXT("[Unit] Hit Mesh=%s"), *GetNameSafe(Mesh));
    if (!Mesh) return;

    UAnimInstance* Raw = Mesh->GetAnimInstance();
    UE_LOG(LogTemp, Warning, TEXT("[Unit] Hit RawAnim=%s Class=%s"),
        *GetNameSafe(Raw),
        Raw ? *Raw->GetClass()->GetName() : TEXT("null"));

    UBattleAnimInstance* Anim = Cast<UBattleAnimInstance>(Raw);
    UE_LOG(LogTemp, Warning, TEXT("[Unit] Hit CastBattleAnim=%s"), *GetNameSafe(Anim));
    if (!Anim) return;

    Anim->RequestHit();

    
}

void ABattleUnitActor::FaceTargetInstant(AActor* Target)
{
    if (!Target) return;
    
    const FVector From = GetActorLocation();
    const FVector To = Target->GetActorLocation();
    FVector Dir = (To - From);
    Dir.Z = 0.f;
    
    if (!Dir.IsNearlyZero())
    {
        const FRotator LookYaw = Dir.Rotation();
        SetActorRotation(FRotator(0.f , LookYaw.Yaw , 0.f));
    }
    
}
void ABattleUnitActor::PushAnimTag(FName Tag)
{
    if (Tag.IsNone()) return;
    PendingAnimTags.Add(Tag); 
    UE_LOG(LogTemp, Warning, TEXT("[Unit] PushAnimTag %s (%s)"),
        *Tag.ToString(), *GetName());
}

void ABattleUnitActor::ConsumeAnimTags()
{
    if (PendingAnimTags.Num() == 0) return;
    TArray<FName> Local = PendingAnimTags;
    PendingAnimTags.Reset();
    
    for (const FName& T : Local)
    {
        HandleAnimTag(T);
    }
    
}


void ABattleUnitActor::HandleAnimTag(FName Tag)
{
    if (!CharacterMeshComp) return;

    UBattleAnimInstance* Anim = Cast<UBattleAnimInstance>(CharacterMeshComp->GetAnimInstance());
    if (!Anim) return;
    
    // 회피 파싱 
    const FString TagStr = Tag.ToString();
    if (TagStr.StartsWith("DodgeOpen_"))
    {
        bDodgeWindowOpen = true;
        bDodgedThisBeat = false;   
        TryConsumeDodgeIntent();   
        return;
    }
    if (Tag == "DodgeEnd")
    {
        bDodgeWindowOpen = false;
        bDodgedThisBeat = false;
        return;
    }
    if (TagStr.StartsWith("DodgeHit_"))
    {
        if (bInvincible)
        {
            // 회피 성공 피드백(이펙트/사운드)만 주고 무시
            return;
        }

        // 기존 Hit 처리로 이어가기
        if (Anim)
        {
            Anim->PlayerHit();
        }
        return;
    }
    
    
    if (Tag == TAG_TurnEnd)
    {
        
        OnTurnEnd();
        return;
    }
    
    if (Tag == TAG_ParryStart)
    {
        Anim->Notify_ParryStart();
        return;
    }
    
    if (Tag == TAG_ParryEnd)
    {
        Anim->Notify_ParryEnd();
        return;
    }
    
    if (Tag == TAG_Retreat)
    {
        StartEnemyRetreat();
        return;
    }
    if (Tag == TAG_HitConsume)
    {
        // 추후 hitrequest 같은 bool 후처리 하는곳
        Anim->ConsumePlayerHit();
        Anim->ConsumeHit();
        
        
        bParryWindowOpen = false;
        bParryPrimedThisBeat = false;
        bDodgedThisBeat = false;
        return;
        
    }
    if (Tag == TAG_End)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Def] End"));
        ResolvePatternEnd();
        ResetPatternState();
    }
    
    // 파싱 위치
    const FString S = Tag.ToString();
    int32 N = 0;
    
    if (ParseIndex(S , PFX_Start , N))
    {
        ResetPatternState();
        BeatTotal  = N; // 총 타수 
        BeatIndex = -1;
        return;
    }
    
    if (ParseIndex(S , PFX_Open , N))
    {
        BeatIndex =N;
        bParryWindowOpen = true;
        bParryPrimedThisBeat =false;
        bDodgedThisBeat = false;
        
        return;
    }
    if (ParseIndex(S,PFX_Hit,N))
    {
        BeatIndex = N;
        
        if (bParryPrimedThisBeat)
        {
            ParrySuccessCount++;
            UE_LOG(LogTemp, Warning, TEXT("[Def] PARRY SUCCESS %d/%d (idx=%d)"),
                ParrySuccessCount, BeatTotal, BeatIndex);
            //TODO > 패링 성공 
        }
        else if (bDodgedThisBeat)
        {
            bUsedDodge = true;
            
            //TODO >> 회피 성공
        }
        else
        {
            bPatternFailed =true;
            UBattleAnimInstance* BattleAnim = Cast<UBattleAnimInstance>(CharacterMeshComp->GetAnimInstance());
            BattleAnim->PlayerHit();
            //TODO >> 회피 , 패링 실패! 피격 !
        }
        bParryWindowOpen = false;
        return;
    }
    
}

void ABattleUnitActor::TryConsumeDodgeIntent()
{
    if (!bDodgeWindowOpen) return;
    if (!bDodgeIntent) return;
    if (bDodgedThisBeat) return;

    bDodgeIntent = false;
    bDodgedThisBeat = true;

    if (UBattleAnimInstance* Anim = Cast<UBattleAnimInstance>(CharacterMeshComp->GetAnimInstance()))
    {
        Anim->RequestDodge(); 
    }

    StartIFrame(0.25f);
}

void ABattleUnitActor::StartIFrame(float Duration)
{
    bInvincible = true;
    GetWorld()->GetTimerManager().ClearTimer(IFrameTimerHandle);
    GetWorld()->GetTimerManager().SetTimer(IFrameTimerHandle,
        this, &ABattleUnitActor::EndIFrame, Duration, false);
}

void ABattleUnitActor::EndIFrame()
{
    bInvincible = false;
}

void ABattleUnitActor::TryParry()
{
    if (!bParryWindowOpen) return;
    if (bParryPrimedThisBeat) return;

    if (UBattleAnimInstance* Anim = Cast<UBattleAnimInstance>(CharacterMeshComp->GetAnimInstance()))
    {
        bool bAccepted = Anim->RequestParry();
        UE_LOG(LogTemp, Warning, TEXT("[Player] TryParry -> RequestParry"));
        if (bAccepted)
        {
            bParryPrimedThisBeat =true;
        }
    };
    

    
    
}

void ABattleUnitActor::TryDodge()
{
    if (!bParryWindowOpen) return;
    if (bParryPrimedThisBeat) return;
    
    bDodgedThisBeat = true;
    DodgeIntentUntilTime = GetWorld()->TimeSeconds + DodgeIntentBufferSeconds;
}

void ABattleUnitActor::ResolvePatternEnd()
{
    const bool bAllParry = 
        (BeatTotal > 0) && (!bPatternFailed) && (!bUsedDodge) && (ParrySuccessCount == BeatTotal);
    
    if (bAllParry)
    {
        // TODO 반격 GOGO
        // 애니매이션 출력 
        
    }
    
}

void ABattleUnitActor::ResetPatternState()
{
    bParryWindowOpen = false;
    bParryPrimedThisBeat = false;
    bDodgedThisBeat = false;
    
    BeatIndex = -1 ;
    BeatTotal = 0;
    ParrySuccessCount = 0;
    
    bParryPrimedThisBeat = false;
    bUsedDodge = false;
    
}



void ABattleUnitActor::CacheCharacterMesh()
{
    if (CharacterMeshComp && IsValid(CharacterMeshComp))
    {
        return;
    }
    // 첫 스켈레탈메시 잡기 
    CharacterMeshComp = FindComponentByClass<USkeletalMeshComponent>();
    
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
