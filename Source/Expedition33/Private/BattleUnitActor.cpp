
#include "BattleUnitActor.h"

#include "BattleGameMode.h"
#include "Components/SkeletalMeshComponent.h"
#include "Battle/BattleAnimInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Battle/BattleTurnManager.h"
#include "Kismet/GameplayStatics.h"

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

static const FName TAG_FXStart(TEXT("SkillImpact"));

static const FString TAG_DodgeOpen(TEXT("DodgeOpen_"));
static const FString TAG_DodgeHit(TEXT("DodgeHit_"));
static const FName TAG_DodgeEnd(TEXT("DodgeEnd"));

static const FName TAG_PhantomCamStart(TEXT("PhantomCamStart"));
static const FName TAG_PhantomImpact(TEXT("PhantomImpact"));
static const FName TAG_PhantomCamEnd(TEXT("PhantomCamEnd"));

static const FName TAG_GommageStart(TEXT("GommageCamStart"));
static const FName TAG_GommageCamFocus(TEXT("GommageCamFocus"));
static const FName TAG_GommageImpact(TEXT("GommageImpact"));
static const FName TAG_GommageCamEnd(TEXT("GommageCamEnd"));
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
    //패링 카운트 초기화
    PendingAnimTags.Reset();
    Tags.Reset();
    
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
        bWalking = false;
        WalkElapsed = 0.f;
      
        
        
        bRetreating = false;
        RetreatElapsed = 0.f;

       
        GetWorldTimerManager().ClearTimer(EnemyPhaseTimer);
        
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
    UE_LOG(LogTemp, Warning, TEXT("[EndTurn] %s Phase=%d"), *GetName(), (int32)EnemyPhaseRuntime);
    PendingAnimTags.Reset();
    
    UE_LOG(LogTemp, Warning, TEXT("[BattleUnitActor] OnTurnEnd called on %s | bIsMyTurn=%d"),
        *GetNameSafe(this), bIsMyTurn ? 1 : 0);

    if (!bIsMyTurn)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BattleUnitActor] OnTurnEnd ignored (already ended) on %s"),
            *GetNameSafe(this));
        return;
    }
    
    
    bIsMyTurn = false;

    UE_LOG(LogTemp, Warning,
        TEXT("[BattleUnitActor] OnTurnEnd"));
    
    if (UnitType == EBattleUnitType::Enemy)
    {
        GetWorldTimerManager().ClearTimer(EnemyPhaseTimer);
        CleanupEnemyTurnArtifacts(); 
        bWalking = false;
        WalkElapsed = 0.f;


        bRetreating = false;
        RetreatElapsed = 0.f;

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

bool ABattleUnitActor::ApplyDamageSpec(const FDamageSpec& Spec, ABattleUnitActor* InstigatorUnit)
{
    if (!bIsAlive) return false;

    // 1) 최종 데미지 계산( 방어/저항/약점 )
    const float Base = (float)Spec.Amount;
    const float Mul  = FMath::Max(0.f, Spec.Multiplier);
    int32 FinalDamage = FMath::Max(0, FMath::RoundToInt(Base * Mul));

    if (FinalDamage <= 0) return false;

    // 2) HP 감소
    const int32 OldHP = CurrentHP;
    CurrentHP = FMath::Clamp(CurrentHP - FinalDamage, 0, MaxHP);



   
    // 5) 데미지 처리
    OnHPChanged.Broadcast(CurrentHP, MaxHP);
    // 6) 사망 처리
    if (CurrentHP <= 0)
    {
        bIsAlive = false;
        TriggerDeath();
        return true;
    }
    // 3) 피격 애니 트리거 
    TriggerHitReaction();
    return true;
   
}

void ABattleUnitActor::TriggerHitReaction()
{
    if (USkeletalMeshComponent* Mesh = GetCharacterMesh())
    {
        if (UBattleAnimInstance* Anim = Cast<UBattleAnimInstance>(Mesh->GetAnimInstance()))
        {
            /*if (NS_HitFX) 
            {
                // 1) 그냥 몸 중앙에
                const FVector Loc = Mesh->GetComponentLocation() + FVector(0,0,80.f);
                UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                    GetWorld(), NS_HitFX, Loc, FRotator::ZeroRotator, FVector(1.f)
                );
            }*/
            Anim->RequestHit();
        }
        
        
    }
    
}

void ABattleUnitActor::TriggerDeath()
{
    UE_LOG(LogTemp, Warning, TEXT("[Death] %s"), *GetName());
    OnDead();
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
    
    FDamageSpec Spec;
    Spec.Source = EBattleActionType::Attack;
    Spec.Amount = 80;
    Spec.Multiplier = 1.f;
    USkeletalMeshComponent* Mesh = GetCharacterMesh();
    
    if (!Mesh) return;
    

    UAnimInstance* Raw = Mesh->GetAnimInstance();
    

    /*UBattleAnimInstance* Anim = Cast<UBattleAnimInstance>(Raw);
    
    if (!Anim) return;*/
    
    
    ApplyDamageSpec(Spec, nullptr);
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

void ABattleUnitActor::OnDead()
{
    if (bDead) return;
    bDead = true;
    
    if (UnitType == EBattleUnitType::Enemy)
    {
        if (UWorld* W = GetWorld())
        {
            if (ABattleGameMode* GM = W->GetAuthGameMode<ABattleGameMode>())
            {
                GM->RequestBattleEnd(true);
            }
        }
    }
    
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
        UE_LOG(LogTemp, Warning, TEXT("[Enemy] Retreat finished -> OnTurnEnd (%s)"), *GetName())
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

    /*// 3) 공격 끝 → Retreat 시작(태그)
    if (Tags.Contains(TAG_Retreat))
    {
        Tags.Remove(TAG_Retreat);
        if (bIsMyTurn && UnitType == EBattleUnitType::Enemy)
        {
            StartEnemyRetreat();
        }
    }*/

    /*
    // 4) 점프백 끝 → 턴 종료(태그)
    if (Tags.Contains(TAG_TurnEnd))
    {
        Tags.Remove(TAG_TurnEnd);
        if (bIsMyTurn)
        {
            OnTurnEnd();
        }
    }
    */
    
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

    
    FDamageSpec Spec;
    Spec.Source = EBattleActionType::FreeAim;
    Spec.Amount = 10;
    Spec.Multiplier = 1.f;
    USkeletalMeshComponent* Mesh = GetCharacterMesh();
    
    if (!Mesh) return;

    UAnimInstance* Raw = Mesh->GetAnimInstance();
    

    /*UBattleAnimInstance* Anim = Cast<UBattleAnimInstance>(Raw);
    
    if (!Anim) return;*/

    ApplyDamageSpec(Spec, nullptr);

    
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

bool ABattleUnitActor::IsEnemyActingNow()
{
    if (UWorld* W = GetWorld())
    {
        if (ABattleGameMode* GM = W->GetAuthGameMode<ABattleGameMode>())
        {
            if (ABattleTurnManager* TM = GM->GetTurnManager())
            {
                if (ABattleUnitActor* Acting = TM->GetCurrentUnit())
                {
                    return Acting->IsEnemyUnit() && Acting->IsMyTurn();
                }
            }
        }
    }
    return false;
}

void ABattleUnitActor::PushAnimTag(FName Tag)
{
    if (Tag.IsNone()) return;
    PendingAnimTags.Add(Tag); 
   
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

    // ------------------------------------------------------------
    // 0) 턴 경계에서 늦게 도착한 태그 필터링 (핵심)
    // ------------------------------------------------------------

    // (A) Enemy의 Retreat/TurnEnd는 "Enemy 자기 턴일 때만" 의미가 있다.
    if (UnitType == EBattleUnitType::Enemy)
    {
        const bool bLateEnemyEndTag =
            (!bIsMyTurn) && (Tag == TAG_Retreat || Tag == TAG_TurnEnd);

        const bool bIsRetreatTag = (Tag == TAG_Retreat);  // EnemyRetreatPending
        const bool bIsTurnEndTag = (Tag == TAG_TurnEnd);  // AttackEndPending

        if (bIsRetreatTag)
        {
            const bool bOk =
                (EnemyPhaseRuntime == EEnemyTurnPhase::AttackFire) ||
                (EnemyPhaseRuntime == EEnemyTurnPhase::AttackIce);

            if (!bOk)
            {
                UE_LOG(LogTemp, Warning, TEXT("[Enemy] Ignore %s (phase=%d)"),
                    *Tag.ToString(), (int32)EnemyPhaseRuntime);
                return;
            }
        }

        if (bIsTurnEndTag)
        {
            const bool bOk = (EnemyPhaseRuntime == EEnemyTurnPhase::Retreat);

            if (!bOk)
            {
                UE_LOG(LogTemp, Warning, TEXT("[Enemy] Ignore %s (phase=%d)"),
                    *Tag.ToString(), (int32)EnemyPhaseRuntime);
                return;
            }
        }
    }

   
    const bool bIsDefOrDodge = IsDefenseOrDodgeTag(Tag);
    ////디버그
    if (bIsDefOrDodge)
    {
        UE_LOG(LogTemp, Warning, TEXT("[onTag] %s | DefOrDodge=1 | EnemyActing=%d | MyTurn=%d | Unit=%s"),
            *Tag.ToString(),
            IsEnemyActingNow()?1:0,
            bIsMyTurn?1:0,
            *GetNameSafe(this)
        );
    }
    
    
    const bool bCleanupTag =
    (Tag == TAG_End) ||           // DefEnd
    (Tag == TAG_HitConsume) ||    // HitConsume
    (Tag == TAG_ParryEnd) ||      // ParryEnd
    (Tag == FName(TEXT("DodgeEnd"))); // DodgeEnd(상수 있으면 그걸로)
    
    if (bIsDefOrDodge && !IsEnemyActingNow() && !bCleanupTag)
    {
        DebugLogIgnoreTag(Tag, TEXT("Defense/Dodge tag but enemy turn not active"));
        return;
    }
    
    
    

    // ------------------------------------------------------------
    // 1) Dodge 파싱
    // ------------------------------------------------------------
    const FString TagStr = Tag.ToString();

    if (TagStr.StartsWith(TEXT("DodgeOpen_")))
    {
        bDodgeWindowOpen = true;
        bDodgedThisBeat = false;
        TryConsumeDodgeIntent();
        return;
    }

    if (Tag == FName(TEXT("DodgeEnd")))
    {
        bDodgeWindowOpen = false;
        bDodgedThisBeat = false;
        return;
    }

    if (TagStr.StartsWith(TEXT("DodgeHit_")))
    {
        if (bInvincible)
        {
            // 회피 성공(무적) -> 피드백만 주고 무시
            return;
        }

        // 회피 실패면 피격
        Anim->PlayerHit();
        return;
    }

    if (Tag == TAG_PhantomCamStart)
    {
        if (ABattlePlayerController* PC = Cast<ABattlePlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
        {
            if (ABattleCameraActor* Cam = PC->GetBattleCam())
            {
                Cam->StartPhantomStrikeCam(this, PendingSkillTarget);
            }
        }
        return;
    }

    if (Tag == TAG_PhantomImpact)
    {
        if (ABattlePlayerController* PC = Cast<ABattlePlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
        {
            if (ABattleCameraActor* Cam = PC->GetBattleCam())
            {
                Cam->PhantomStrikeImpactBeat();
            }
        }
        // FX 를 여기서 해도 되나 타격당으로 하려고 따로 태그를 빼두었음
        //  OnSkillImpact();
        return;
    }
    if (Tag == TAG_PhantomCamEnd)
    {
        if (ABattlePlayerController* PC = Cast<ABattlePlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
        {
            if (ABattleCameraActor* Cam = PC->GetBattleCam())
            {
                Cam->EndPhantomStrikeCam();
            }
        }
        return;
    }
    
    if ( Tag == TAG_GommageStart)
    {
        if (ABattlePlayerController* PC = Cast<ABattlePlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
        {
            if (ABattleCameraActor* Cam = PC->GetBattleCam())
            {
                Cam->StartGommageCam(this , PendingSkillTarget);
            }
        }
        return;
    }
    if ( Tag == TAG_GommageImpact)
    {
        if (ABattlePlayerController* PC = Cast<ABattlePlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
        {
            if (ABattleCameraActor* Cam = PC->GetBattleCam())
            {
                Cam->GommageImpactBeat();
            }
        }
        return;
    }
    if ( Tag == TAG_GommageCamEnd)
    {
        if (ABattlePlayerController* PC = Cast<ABattlePlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
        {
            if (ABattleCameraActor* Cam = PC->GetBattleCam())
            {
                Cam->EndGommageCam();
            }
        }
        return;
    }
    
    
    // ------------------------------------------------------------
    // 2) 공통 태그
    // ------------------------------------------------------------
    if (Tag == TAG_TurnEnd)
    {
        // Enemy/Player 모두: 자기 턴일 때만 종료
        if (!bIsMyTurn)
        {
            DebugLogIgnoreTag(Tag, TEXT("TurnEnd but not my turn"));
            return;
        }

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
        // Enemy만 의미 있음 + Enemy 턴일 때만
        if (UnitType == EBattleUnitType::Enemy)
        {
            if (!bIsMyTurn)
            {
               
                return;
            }

            StartEnemyRetreat();
        }
        return;
    }

    if (Tag == TAG_HitConsume)
    {
        Anim->ConsumePlayerHit();
        Anim->ConsumeHit();

        bParryWindowOpen = false;
        bParryPrimedThisBeat = false;
        bDodgedThisBeat = false;
        bDodgeWindowOpen = false;

        return;
    }

    if (Tag == TAG_End)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Def] End"));

        // DefEnd는 “적 턴이 끝난 뒤 늦게 오면” 위에서 이미 필터링됨
        ResetPatternState();
        if (Anim)
        {
            Anim->bParryPlaying = false;
            Anim->bParryRequest = false;
      //      Anim->ParryRequestHoldFrames = 0;
        }
        return;
    }
    if (Tag == TAG_FXStart)
    {
        OnSkillImpact();
        UE_LOG(LogTemp , Warning , TEXT("[FX]  FX : START "))
        return;
    }
    
    
        //카운터
    if (Tag == FName("CounterStart"))
    {
        Anim->Notify_CounterStart();
        return;
    }
    if (Tag == FName("CounterEnd"))
    {
        Anim->Notify_CounterEnd();
        return;
    }

    // ------------------------------------------------------------
    // 3) DefStart/DefOpen/DefHit 파싱 (패링/닷지 판정)
    // ------------------------------------------------------------
    int32 N = 0;

    if (ParseIndex(TagStr, PFX_Start, N))
    {
        ResetPatternState();
        BeatTotal = N;
        BeatIndex = -1;
        ParrySuccessCount = 0;
        return;
    }

    if (ParseIndex(TagStr, PFX_Open, N))
    {
        BeatIndex = N;
        bParryWindowOpen = true;
        bParryPrimedThisBeat = false;
        bDodgedThisBeat = false;
        UE_LOG(LogTemp, Warning, TEXT("[DefOpen] idx=%d | Win=1 | BeatTotal=%d | Unit=%s"),
        N, BeatTotal, *GetNameSafe(this));
        
        return;
    }

    if (ParseIndex(TagStr, PFX_Hit, N))
    {
        BeatIndex = N;

        // 1) 판정
        if (bParryPrimedThisBeat)
        {
            ParrySuccessCount++;

            UE_LOG(LogTemp, Warning, TEXT("[Def] PARRY SUCCESS %d/%d (idx=%d)"),
                ParrySuccessCount, BeatTotal, BeatIndex);

            // 2) 마지막 히트에서 올패링이면 즉시 카운터 + 적 턴 강제 종료
            const bool bAllParryNow =
                (BeatTotal > 0) &&
                (!bPatternFailed) &&
                (!bUsedDodge) &&
                (ParrySuccessCount == BeatTotal) &&
                (N == BeatTotal);

            if (bAllParryNow && !bCounterTriggeredThisPattern)
            {
                bCounterTriggeredThisPattern = true;

                UE_LOG(LogTemp, Warning, TEXT("[Counter] ALL PARRY NOW @DefHit_%d -> Trigger Counter"), N);

                // 반격 애니
                Anim->RequestCounter();

                // 방어 윈도우 강제 닫기(늦은 DefOpen/DefHit가 와도 위 필터가 막지만 안전하게)
                bParryWindowOpen = false;
                bParryPrimedThisBeat = false;
                bDodgeWindowOpen = false;
                bDodgedThisBeat = false;

                // 현재 행동자(적) 강제 종료
                if (UWorld* W = GetWorld())
                {
                    if (ABattleGameMode* GM = W->GetAuthGameMode<ABattleGameMode>())
                    {
                        if (ABattleTurnManager* TM = GM->GetTurnManager())
                        {
                            if (ABattleUnitActor* Acting = TM->GetCurrentUnit())
                            {
                                if (Acting && Acting->UnitType == EBattleUnitType::Enemy)
                                {
                                    
                                    if (UBattleAnimInstance* EAnim = Cast<UBattleAnimInstance>(Acting->GetCharacterMesh()->GetAnimInstance()))
                                    {
                                        EAnim->ForceEnemyIdle();
                                    }
                                    Acting->SetEnemyPhaseOnAnim(EEnemyTurnPhase::Idle);
                                    Acting->OnTurnEnd();
                                }
                                
                                
                            }
                        }
                    }
                }
            }
        }
        else if (bDodgedThisBeat)
        {
            bUsedDodge = true;
            // dodge 성공 처리(필요하면 여기서 이펙트)
        }
        else
        {
            bPatternFailed = true;
            Anim->PlayerHit();
        }

        // beat 종료 처리
        bParryPrimedThisBeat = false;
        bParryWindowOpen = false;
        bDodgedThisBeat = false;

        
        UE_LOG(LogTemp, Warning, TEXT("[DefHitEnd] idx=%d | Win=%d Primed=%d"),
        N, bParryWindowOpen?1:0, bParryPrimedThisBeat?1:0);
        return;
    }

    // ------------------------------------------------------------
    // 4) 카운터 카메라 태그
    // ------------------------------------------------------------
    if (Tag == FName("CounterCamStart"))
    {
        if (APlayerController* PC0 = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
        {
            if (ABattlePlayerController* BPC = Cast<ABattlePlayerController>(PC0))
            {
                ABattleUnitActor* Enemy = nullptr;

                if (UWorld* W = GetWorld())
                {
                    if (ABattleGameMode* GM = W->GetAuthGameMode<ABattleGameMode>())
                    {
                        if (ABattleTurnManager* TM = GM->GetTurnManager())
                        {
                            Enemy = TM->GetCurrentUnit();
                        }
                    }
                }

                BPC->StartCounterCamera(this, Enemy);
            }
        }
        return;
    }

    if (Tag == FName("CounterImpact"))
    {
        if (APlayerController* PC0 = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
        {
            if (ABattlePlayerController* BPC = Cast<ABattlePlayerController>(PC0))
            {
                BPC->TriggerCounterImpact();
            }
        }
        return;
    }

    if (Tag == FName("CounterCamEnd"))
    {
        if (APlayerController* PC0 = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
        {
            if (ABattlePlayerController* BPC = Cast<ABattlePlayerController>(PC0))
            {
                BPC->EndCounterCamera(true);
            }
        }
        return;
    }
    
}

bool ABattleUnitActor::IsEnemyTurnActive() const
{
    if (UWorld* W = GetWorld())
    {
        if (ABattleGameMode* GM = W->GetAuthGameMode<ABattleGameMode>())
        {
            if (ABattleTurnManager* TM = GM->GetTurnManager())
            {
                if (ABattleUnitActor* Cur = TM->GetCurrentUnit())
                {
                    // “현재 행동자”가 적이고, 적이 자기 턴으로 살아있을 때만 true
                    return (Cur->UnitType == EBattleUnitType::Enemy) && Cur->bIsMyTurn;
                }
            }
        }
    }
    return false;;
}

bool ABattleUnitActor::IsDefenseOrDodgeTag(const FName& Tag) const
{
    const FString S = Tag.ToString();

    const bool bDef =
        S.StartsWith(TEXT("DefStart_")) ||
        S.StartsWith(TEXT("DefOpen_"))  ||
        S.StartsWith(TEXT("DefHit_"))   ||
        (Tag == FName(TEXT("DefEnd")));
 
    const bool bDodge =
        S.StartsWith(TEXT("DodgeOpen_")) ||
        S.StartsWith(TEXT("DodgeHit_"))  ||
        (Tag == FName(TEXT("DodgeEnd")));

    return bDef || bDodge;
}

void ABattleUnitActor::CleanupEnemyTurnArtifacts()
{
    GetWorldTimerManager().ClearTimer(EnemyPhaseTimer);

    bWalking = false;
    WalkElapsed = 0.f;

    bRetreating = false;
    RetreatElapsed = 0.f;

    // 남아있는 태그 큐 폐기 (늦게 온 태그가 다음 프레임에 또 먹히는 것 방지)
    PendingAnimTags.Reset();

    // 애니도 idle로 정리(원위치 스냅은 필요에 따라)
    SetEnemyPhaseOnAnim(EEnemyTurnPhase::Idle);
    SetActorLocation(EnemyHomeLocation);
}

bool ABattleUnitActor::CanUseSkill(int32 SkillIndex) const
{
    if (SkillIndex < 0 || SkillIndex > 1) return false;
    
    if (SkillIndex == 0 )
    {
        if (Cost < 5)
        {
            return false;
        }
        return true;
    }
    
    if (SkillIndex == 1 )
    {
        if (Cost < 7)
        {
            return false;
        }
        return true;
    }
    
    // todo : 코스트 
    
    return true;
}

void ABattleUnitActor::UseSkill(int32 SkillIndex, ABattleUnitActor* Target)
{
    if (!CanUseSkill(SkillIndex)|| !Target) return;
    
    PendingSkillIndex = SkillIndex;
    PendingSkillTarget = Target;
   // USkeletalMeshComponent* Mesh = GetCharacterMesh();
    
    if (UBattleAnimInstance* Anim = Cast<UBattleAnimInstance>(CharacterMeshComp->GetAnimInstance()))
    {
        
        Anim->RequestSkill(SkillIndex);
    }
    
}

void ABattleUnitActor::OnSkillImpact()
{
    /*UE_LOG(LogTemp, Warning, TEXT("[FX]Skill Impact"));

    if (!PendingSkillTarget || !NS_HitFX) return;

    const FVector Loc = PendingSkillTarget->GetActorLocation() + HitFXOffset;

    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(), NS_HitFX, Loc, FRotator::ZeroRotator, FVector(1.f)
    );*/
    
    UE_LOG(LogTemp, Warning, TEXT("[FX] Owner=%s Class=%s PendingTarget=%s NS_HitFX=%s"),
       *GetName(),
       *GetClass()->GetName(),
       PendingSkillTarget ? *PendingSkillTarget->GetName() : TEXT("NULL"),
       NS_HitFX ? *NS_HitFX->GetName() : TEXT("NULL")
   );
    if (!PendingSkillTarget) return;

    // 타겟(피격자)의 FX를 사용
    UNiagaraSystem* FX = PendingSkillTarget->NS_HitFX;  
    if (!FX) return;

    const FVector Loc = PendingSkillTarget->GetActorLocation() + HitFXOffset;

    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(), FX, Loc, FRotator::ZeroRotator, FVector(1.f)
    );
    
    
    
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
        UE_LOG(LogTemp, Warning, TEXT("[Tag ParryStart] Win=%d  Primed=%d"),
        bParryWindowOpen?1:0,
       
        bParryPrimedThisBeat?1:0
    );
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
    
    if (UBattleAnimInstance* Anim = Cast<UBattleAnimInstance>(CharacterMeshComp->GetAnimInstance()))
    {
        const bool bAccepted = Anim->RequestDodge();
        UE_LOG(LogTemp, Warning, TEXT("[Player] TryDodge accepted=%d"), bAccepted ? 1 : 0);

        if (bAccepted)
        {
            bDodgedThisBeat = true;
            bUsedDodge = true;              
            // StartIFrame(0.25f);          
        }
    }
}

/*
void ABattleUnitActor::ResolvePatternEnd()
{
    const bool bAllParry = 
        (BeatTotal > 0) && (!bPatternFailed) && (!bUsedDodge) && (ParrySuccessCount == BeatTotal);
    
    if (!bAllParry) return;
    
    
    if (CharacterMeshComp)
    {
        if (UBattleAnimInstance* Anim = Cast<UBattleAnimInstance>(CharacterMeshComp->GetAnimInstance()))
        {
            Anim->RequestCounter();
        }
    }

    
    if (UWorld* W = GetWorld())
    {
        if (ABattleGameMode* GM = W->GetAuthGameMode<ABattleGameMode>())
        {
            if (ABattleTurnManager* TM = GM->GetTurnManager())
            {
                if (ABattleUnitActor* Current = TM->GetCurrentUnit())
                {
                    if (Current->UnitType == EBattleUnitType::Enemy)
                    {
                        Current->OnTurnEnd(); // 적이 자기 턴을 끝내게 해야
                    }
                }
            }
        }
    }
    
}
*/

void ABattleUnitActor::ResetPatternState()
{
    bParryWindowOpen = false;
    bParryPrimedThisBeat = false;
    bDodgedThisBeat = false;
    bPatternFailed  = false;
    bCounterTriggeredThisPattern = false;
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

void ABattleUnitActor::ResetDefenseState()
{
    bParryWindowOpen = false;
    
}

void ABattleUnitActor::DebugLogIgnoreTag(const FName& Tag, const TCHAR* Reason) const
{
    UE_LOG(LogTemp, Warning, TEXT("[TagIgnore] %s | Tag=%s | Unit=%s | MyTurn=%d"),
        Reason, *Tag.ToString(), *GetNameSafe(this), bIsMyTurn ? 1 : 0);

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
    if (!bIsMyTurn) return;
    GetWorld()->GetTimerManager().ClearTimer(EnemyPhaseTimer);

    const bool bFire = (PlannedAttack == EEnemyAttackType::Fire);
    SetEnemyPhaseOnAnim(bFire ? EEnemyTurnPhase::AttackFire : EEnemyTurnPhase::AttackIce);

    UE_LOG(LogTemp, Warning, TEXT("[Enemy] StartAttack=%s"), bFire ? TEXT("Fire") : TEXT("Ice"));

    // 여기서 턴 종료하지 않음
    // 공격 애니 끝(Notify_BattleTag) → EnemyRetreatPending 태그 → StartEnemyRetreat()
}

void ABattleUnitActor::StartEnemyRetreat()
{
    UE_LOG(LogTemp, Warning, TEXT("[Enemy] StartEnemyRetreat | MyTurn=%d"), bIsMyTurn?1:0);
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
       // UE_LOG(LogTemp,Warning , TEXT("[ENEMY] :: SetEnemyPhase"))
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

