// Fill out your copyright notice in the Description page of Project Settings.


#include "Battle/BattlePlayerController.h"
#include "BattleGameMode.h"
#include "Battle/BattleTurnManager.h"
#include "BattleUnitActor.h"

#include "Battle/BattleAnimInstance.h"
#include "Battle/BattleCameraActor.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/CinematicCamera/Public/CineCameraActor.h"

class UCineCameraComponent;
static float EaseOutCubic(float t)
{ return 1.f - FMath::Pow(1.f - t, 3.f); }
void ABattlePlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    check(InputComponent);

    // 키 바인딩 기본 UI
    InputComponent->BindKey(EKeys::S, IE_Pressed, this, &ABattlePlayerController::OnPressS_AttackMode);
    InputComponent->BindKey(EKeys::A, IE_Pressed, this, &ABattlePlayerController::OnPressA_PrevTarget);
    InputComponent->BindKey(EKeys::D, IE_Pressed, this, &ABattlePlayerController::OnPressD_NextTarget);
    InputComponent->BindKey(EKeys::F, IE_Pressed, this, &ABattlePlayerController::OnPressF_ConfirmTarget);
    // 키바인딩 FreeAim
    InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ABattlePlayerController::OnFreeAimFire);
    InputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &ABattlePlayerController::OnFreeAimStart);
    InputComponent->BindKey(EKeys::RightMouseButton, IE_Released, this, &ABattlePlayerController::OnFreeAimEnd);
    // 키바인딩 E >> 패링
    InputComponent->BindKey(EKeys::E , IE_Released, this, &ABattlePlayerController::OnParry);
    InputComponent->BindKey(EKeys::Q,IE_Pressed, this, &ABattlePlayerController::OnDodge);
}

ABattleCameraActor* ABattlePlayerController::GetBattleCam()
{
    if (BattleCam && IsValid(BattleCam)) return BattleCam;

    // 지금 보고 있는 카메라가 곧 배틀카메라임
    BattleCam = Cast<ABattleCameraActor>(GetViewTarget());
    if (!BattleCam)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PC] GetBattleCam failed. ViewTarget=%s"),
            *GetNameSafe(GetViewTarget()));
    }
    return BattleCam;
}


bool ABattlePlayerController::IsPlayerTurn() const
{
    ABattleUnitActor* Unit = GetCurrentUnit();
    return Unit && Unit->IsMyTurn() && Unit->IsPlayerUnit(); 
}

ABattleUnitActor* ABattlePlayerController::GetCurrentUnit() const
{
    if (UWorld* World = GetWorld())
    {
        if (ABattleGameMode* GM = World->GetAuthGameMode<ABattleGameMode>())
        {
            if (ABattleTurnManager* TM = GM->GetTurnManager())
            {
                return TM->GetCurrentUnit();
            }
        }
    }
    return nullptr;
}

void ABattlePlayerController::CacheEnemies()
{
    CachedEnemies.Reset();

  
    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABattleUnitActor::StaticClass(), Found);

    for (AActor* A : Found)
    {
        ABattleUnitActor* Unit = Cast<ABattleUnitActor>(A);
        if (!Unit) continue;
        if (Unit->IsEnemyUnit() && Unit->IsAlive()) 
        {
            CachedEnemies.Add(Unit);
        }
    }
}

void ABattlePlayerController::UpdateCounterCam_Cinematic(float DeltaTime)
{
    ABattleCameraActor* CamActor = BattleCam;
    
    if (!CamActor)
    {
        CamActor = Cast<ABattleCameraActor>(GetViewTarget());
        
        return;
    }
    if (!CamActor) return;

    const FVector P = CounterPlayer->GetActorLocation();
    const FVector E = CounterEnemy->GetActorLocation();

    // --- Direction / Side
    FVector Dir = (E - P); Dir.Z = 0.f;
    if (Dir.IsNearlyZero()) Dir = CounterPlayer->GetActorForwardVector();
    Dir = Dir.GetSafeNormal();

    const FVector Side = FVector::CrossProduct(FVector::UpVector, Dir).GetSafeNormal();

    // --- Pivot(중간점 + 약간 위)
    FVector Pivot = (P + E) * 0.5f;
    Pivot.Z += PivotHeight;

    // --- LookAt: 적을 더 크게 잡기(원작 감성)
    FVector LookAt = (E * 0.7f + P * 0.3f); 
    LookAt.Z += CounterLookAtZ;

    auto Smooth01 = [](float X)
    {
        X = FMath::Clamp(X, 0.f, 1.f);
        return X * X * (3.f - 2.f * X); // smoothstep
    };

    const float T = CounterElapsed;

    // ---- Stage timing
    const float T_IntroEnd  = 0.28f;  // 빠르게 자리잡기
    const float T_WideHold  = 0.75f;  // 와이드 홀드
    const float T_PunchEnd  = 0.95f;  // 펀치 인
    // 나머지는 여운/복귀

    // ---- Parameters
    float Dist    = 280.f;
    float SideAmt = 140.f;
    float Height  = 90.f;
    float FOV     = 40.f;

    // Start(가까운 느낌) -> Wide(더 멀리)
    const float StartDist = StartDistance; 
    const float WideDist  = WideDistance; 
    const float PunchDist = PunchDistance;

    const float StartSide = StartSides;     
    const float WideSide  = WideSides;      
    const float PunchSide = PunchSides;     

    const float StartF = StartFOV;        
    const float WideF  = WideFOV;          
    const float PunchF = PunchFOV;         
    const float RecoverF = RecoverFOV;    

    if (T < T_IntroEnd)
    {
        float a = Smooth01(T / T_IntroEnd);
        Dist    = FMath::Lerp(StartDist, WideDist, a);
        SideAmt = FMath::Lerp(StartSide, WideSide, a);
        FOV     = FMath::Lerp(StartF,    WideF,    a);
    }
    else if (T < T_WideHold)
    {
        Dist    = WideDist;
        SideAmt = WideSide;
        FOV     = WideF;
    }
    else if (T < T_PunchEnd)
    {
        float a = Smooth01((T - T_WideHold) / (T_PunchEnd - T_WideHold));
        Dist    = FMath::Lerp(WideDist,  PunchDist, a);
        SideAmt = FMath::Lerp(WideSide,  PunchSide, a);
        FOV     = FMath::Lerp(WideF,     PunchF,    a);

        if (!bImpactTriggered)
        {
            bImpactTriggered = true;
            TriggerCounterImpact(); // 임팩트 슬로우/쉐이크
        }
    }
    else
    {
        // 여운: 살짝 뒤로 + FOV 복구
        float a = Smooth01(FMath::Clamp((T - T_PunchEnd) / 0.35f, 0.f, 1.f));
        Dist    = FMath::Lerp(PunchDist, PunchDist + 25.f, a);
        SideAmt = PunchSide;
        FOV     = FMath::Lerp(PunchF,    RecoverF,        a);
    }

    // ---- Orbit
    const float OrbitRad = FMath::DegreesToRadians(OrbitDegrees);
    const float OrbitA = Smooth01(FMath::Clamp((T - 0.1f) / 0.6f, 0.f, 1.f));
    const FVector SideRot =
        (Side * FMath::Cos(OrbitRad * OrbitA) + (-Dir) * FMath::Sin(OrbitRad * OrbitA)).GetSafeNormal();

    const FVector CamLoc = Pivot + (-Dir * Dist) + (SideRot * SideAmt) + FVector(0,0,HeightOffset + Height);
    const FRotator CamRot = (LookAt - CamLoc).Rotation();

    // ---- 위치는 약간만 스무딩
    const float PosInterp = 10.f;  // 8~14 추천
    const float RotInterp = 12.f;  // 10~16 추천

    CamActor->SetActorLocation(
        FMath::VInterpTo(CamActor->GetActorLocation(), CamLoc, DeltaTime, PosInterp)
    );
    CamActor->SetActorRotation(
        FMath::RInterpTo(CamActor->GetActorRotation(), CamRot, DeltaTime, RotInterp)
    );

    if (UCameraComponent* Cine = CamActor->GetCameraComponent())
    {
        Cine->SetFieldOfView(FOV);
        //  여기서 FocusTarget = CounterEnemy 등으로
    }
    
}

void ABattlePlayerController::ClearEnemySelection()
{
    for (ABattleUnitActor* E : CachedEnemies)
    {
        if (E) E->SetSelected(false);
    }
    
        
    
}

void ABattlePlayerController::ApplyEnemySelection()
{
    if (CachedEnemies.Num() == 0) return;
    SelectedEnemyIndex = (SelectedEnemyIndex + CachedEnemies.Num()) % CachedEnemies.Num();

    ClearEnemySelection();
    ABattleUnitActor* NewTarget = CachedEnemies[SelectedEnemyIndex];
    NewTarget->SetSelected(true);
    UE_LOG(LogTemp, Warning, TEXT("[PC] Target Selected = %s"), *NewTarget->GetName());

    //  타겟선택 모드에서만 카메라 연출
    if (InputMode == EBattleInputMode::TargetSelect)
    {
        if (ABattleCameraActor* Cam = GetBattleCam())
        {
            Cam->SetTargetSelect(true, NewTarget);
        }
    }
    
    
    /*if (CachedEnemies[SelectedEnemyIndex])
    {
        CachedEnemies[SelectedEnemyIndex]->SetSelected(true);
        UE_LOG(LogTemp, Warning, TEXT("[PC] Target Selected = %s"), *CachedEnemies[SelectedEnemyIndex]->GetName());
    }*/
}

void ABattlePlayerController::OnPressS_AttackMode()
{
    if (!IsPlayerTurn()) return;
    if (InputMode == EBattleInputMode::FreeAim) return;

    ABattleCameraActor* Cam = GetBattleCam();
    UE_LOG(LogTemp, Warning, TEXT("[PC] S Pressed Cam=%s ViewTarget=%s"),
        *GetNameSafe(Cam), *GetNameSafe(GetViewTarget()));

    if (InputMode == EBattleInputMode::TargetSelect)
    {
        InputMode = EBattleInputMode::None;
        ClearEnemySelection();
        if (Cam) Cam->SetTargetSelect(false, nullptr);
        UE_LOG(LogTemp, Warning, TEXT("[PC] Attack Select Mode OFF"));
        return;
    }

    CacheEnemies();
    if (CachedEnemies.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PC] No Enemies to Select"));
        return;
    }

    InputMode = EBattleInputMode::TargetSelect;
    SelectedEnemyIndex = 0;

    ApplyEnemySelection(); // 여기서 SetTargetSelect(true,target) 들어감
    UE_LOG(LogTemp, Warning, TEXT("[PC] Attack Select Mode ON"));
}

void ABattlePlayerController::OnPressA_PrevTarget()
{
    
    if (InputMode == EBattleInputMode::FreeAim) return;
    if (InputMode != EBattleInputMode::TargetSelect) return;
    
    SelectedEnemyIndex--;
    ApplyEnemySelection();
}

void ABattlePlayerController::OnPressD_NextTarget()
{
    
    if (InputMode == EBattleInputMode::FreeAim) return;
    if (InputMode != EBattleInputMode::TargetSelect) return;
    SelectedEnemyIndex++;
    ApplyEnemySelection();
}

void ABattlePlayerController::OnPressF_ConfirmTarget()
{
    
    if (InputMode == EBattleInputMode::FreeAim) return;
    if (InputMode != EBattleInputMode::TargetSelect) return;
    if (!IsPlayerTurn()) return;
    if (CachedEnemies.Num() == 0) return;

    ABattleUnitActor* Attacker = GetCurrentUnit();
    ABattleUnitActor* Target = CachedEnemies[SelectedEnemyIndex];

    if (!Attacker || !Target) return;

    if (ABattleCameraActor* Cam = GetBattleCam())
    {
        Cam->SetTargetSelect(false, nullptr);   // 내부에서 CamMode=Battle 로 바꾸도록
        
    }
    
    // 확정: 선택 해제 + 모드 종료
    ClearEnemySelection();
    InputMode = EBattleInputMode::None;

    UE_LOG(LogTemp, Warning, TEXT("[PC] Confirm Target=%s -> RequestAttack"), *Target->GetName());
    // 타겟 위치 찾기
    Attacker->SetCurrentTarget(Target); 
    //타겟 바라보기
    Attacker->FaceTargetInstant(Target);
    //공격 
    Attacker->RequestAttack();          
}

void ABattlePlayerController::OnFreeAimStart()
{
    if (!IsPlayerTurn()) return;
    UE_LOG(LogTemp, Warning, TEXT("[PC] ViewTarget=%s"), *GetNameSafe(GetViewTarget()));
    if (InputMode == EBattleInputMode::TargetSelect)
    {
        ClearEnemySelection();
        if (BattleCam) BattleCam->SetTargetSelect(false, nullptr);
    }
    InputMode = EBattleInputMode::FreeAim;
    AimedUnit = nullptr;
    
    
    
    if (ABattleCameraActor* Cam = GetBattleCam())
    {
        Cam->SetFreeAim(true);
        if (ABattleUnitActor* Attacker = GetCurrentUnit())
            Cam->SetAnchor(Attacker);
    }
    
    if (!FreeAimCrosshairWidget && FreeAimCrosshairClass)
    {
        FreeAimCrosshairWidget = CreateWidget<UUserWidget>(this, FreeAimCrosshairClass);
        if (FreeAimCrosshairWidget)
            FreeAimCrosshairWidget->AddToViewport(50);
    }
    
    if (ABattleUnitActor* Attacker = GetCurrentUnit())
    {
        if (USkeletalMeshComponent* Mesh = Attacker->GetCharacterMesh())
        {
            if (UBattleAnimInstance* Anim = Cast<UBattleAnimInstance>(Mesh->GetAnimInstance()))
            {
                Anim->SetFreeAim(true);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[PC] FreeAim ON"));
    
    
}

void ABattlePlayerController::OnFreeAimEnd()
{
    if (InputMode != EBattleInputMode::FreeAim) return;
    ClearAimHighlight();
    InputMode = EBattleInputMode::None;
    AimedUnit = nullptr;
    
    // 1) 카메라 FreeAim OFF
    if (ABattleCameraActor* Cam = GetBattleCam())
    {
        Cam->SetFreeAim(false);
    }
    
    
    if (ABattleUnitActor* Attacker = GetCurrentUnit())
    {
        if (USkeletalMeshComponent* Mesh = Attacker->GetCharacterMesh())
        {
            if (UBattleAnimInstance* Anim = Cast<UBattleAnimInstance>(Mesh->GetAnimInstance()))
            {
                Anim->SetFreeAim(false);
            }
        }
    }
    
    if (FreeAimCrosshairWidget)
    {
        FreeAimCrosshairWidget->RemoveFromParent();
        FreeAimCrosshairWidget = nullptr;
    }

    UE_LOG(LogTemp, Warning, TEXT("[PC] FreeAim OFF"));
}

void ABattlePlayerController::OnFreeAimFire()
{
    
    if (InputMode != EBattleInputMode::FreeAim) return;
    if (!IsPlayerTurn()) return;
   
    ABattleUnitActor* Attacker = GetCurrentUnit();
    if (!Attacker) return;

    //  1발당 1코스트 소비
    if (!Attacker->SpendCost(FreeAimShotCost))
    {
        UE_LOG(LogTemp, Warning, TEXT("[PC] Not enough Cost for FreeAimShot"));
        return;
    }

    UpdateAimTarget();
    ABattleUnitActor* Target = AimedUnit.Get();
    if (!Target)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PC] FreeAimFire: no target"));
        return;
    }
    Target->OnFreeAimHit(LastAimHit.BoneName);
    
   
    
    //  발사 애니 트리거(FreeAimShoot 상태로)
    if (USkeletalMeshComponent* Mesh = Attacker->GetCharacterMesh())
    {
        if (UBattleAnimInstance* Anim = Cast<UBattleAnimInstance>(Mesh->GetAnimInstance()))
        {
            UE_LOG(LogTemp, Warning, TEXT("[PC] ShootRequest sent. FreeAim=%d ShootReq=%d"),
        Anim->bFreeAim ? 1 : 0,
        Anim->bFreeAimShootRequest ? 1 : 0);
            Anim->RequestFreeAimShoot();
        }
    }
    


    // TODO: 데미지는 나중에 여기서 연결
    // - HitBone / WeakPoint면 배율
    // - ImpactPoint에 FX
}

void ABattlePlayerController::OnParry()
{
    ABattleUnitActor* PlayerUnit = GetPlayerUnit();
    
    if (!PlayerUnit || !PlayerUnit->IsAlive()) return;
    
    if (!PlayerUnit->bParryWindowOpen) return;
    
    PlayerUnit->TryParry();

}

void ABattlePlayerController::OnDodge()
{
    
    ABattleUnitActor* PlayerUnit = GetPlayerUnit();
    if (!PlayerUnit || !PlayerUnit->IsAlive()) return;

    if (!PlayerUnit->bParryWindowOpen)
    {
        return; 
    }
    PlayerUnit->TryDodge();
}

ABattleUnitActor* ABattlePlayerController::GetPlayerUnit() const
{
    if (UWorld* World = GetWorld())
    {
        if (ABattleGameMode* GM = World->GetAuthGameMode<ABattleGameMode>())
        {
            return GM->GetBattlePlayerUnit();
        }
    }
    
    return nullptr;
}

void ABattlePlayerController::PlayerTick(float DeltaTime)
{
    Super::PlayerTick(DeltaTime);
    
    if (bCounterCamActive)
    {
        if (InputMode == EBattleInputMode::FreeAim)
        {
            OnFreeAimEnd();
        }

        CounterElapsed += DeltaTime;

        UpdateCounterCam_Cinematic(DeltaTime);

        if (CounterElapsed >= CounterDuration)
        {
            EndCounterCamera(true);
        }
        return;
    }
    
    /*
    if (bCounterCamActive)
{   
    // FreeAim 켜져 있으면 강제 종료(카운터 카메라에 방해됨)
    if (InputMode == EBattleInputMode::FreeAim)
    {
        OnFreeAimEnd();
    }

    // CounterCam 업데이트
    CounterElapsed += DeltaTime;

    // 안전 체크
    /*if (!CounterCam || !CounterPlayer || !CounterEnemy)
    {
        EndCounterCamera(true);
        return;
    }#1#

    const FVector P = CounterPlayer->GetActorLocation();
    const FVector E = CounterEnemy->GetActorLocation();

    const FVector DirPE = (E - P).GetSafeNormal();
    const FVector Side  = FVector::CrossProduct(FVector::UpVector, DirPE).GetSafeNormal();

    const FVector Pivot = (P + E) * 0.5f + FVector(0,0,PivotHeight);

    auto Smooth01 = [](float X)
    {
        X = FMath::Clamp(X, 0.f, 1.f);
        return X * X * (3.f - 2.f * X);
    };
        UpdateCounterCam();
    
        float Dist = WideDistance;
    
        float SideAmt = WideSide;
    
        float FOV = WideFOV;

    
    // 0.00 ~ 1.36 : pull-back (Start -> Wide)
    // 1.36 ~ 2.00 : hold wide
    // 2.00 ~ 2.20 : punch-in (Wide -> Punch) + FOV 킥
    // 2.20 ~ 3.00 : recover (Punch -> slight back) + FOV 복구
    // 3.00 ~ end  : 복귀
    const float T = CounterElapsed;

    if (T < 1.36f)
    {
        float a = Smooth01(T / 1.36f);
        Dist    = FMath::Lerp(StartDistance, WideDistance, a);
        SideAmt = FMath::Lerp(StartSide,    WideSide,    a);
        FOV     = FMath::Lerp(StartFOV,     WideFOV,     a);
    }
    else if (T < 2.00f)
    {
        Dist    = WideDistance;
        SideAmt = WideSide;
        FOV     = WideFOV;
    }
    else if (T < 2.20f)
    {
        float a = Smooth01((T - 2.00f) / 0.20f);
        Dist    = FMath::Lerp(WideDistance,  PunchDistance, a);
        SideAmt = FMath::Lerp(WideSide,      PunchSide,     a);
        FOV     = FMath::Lerp(WideFOV,       PunchFOV,      a);

        // 자동 임팩트(원하면 TriggerCounterImpact 태그로만 하게 바꿔도 됨)
        if (!bImpactTriggered)
        {
            bImpactTriggered = true;
            TriggerCounterImpact();
        }
    }
    else if (T < 3.00f)
    {
        float a = Smooth01((T - 2.20f) / 0.80f);
        Dist    = FMath::Lerp(PunchDistance, PunchDistance + 25.f, a);
        SideAmt = PunchSide;
        FOV     = FMath::Lerp(PunchFOV, RecoverFOV, a);
    }
    else if (T < CounterDuration)
    {
        // 끝부분 유지
        Dist = PunchDistance + 25.f;
        SideAmt = PunchSide;
        FOV = RecoverFOV;
    }
    else
    {
        EndCounterCamera(true);
        return;
    }

    // 오비트(살짝 회전) 느낌: Side를 Dir와 섞어서 비틀기
    const float OrbitRad = FMath::DegreesToRadians(OrbitDegrees);
    const float BlendOrbit = Smooth01(FMath::Clamp((T - 0.2f) / 0.8f, 0.f, 1.f));
    const FVector SideRot = (Side * FMath::Cos(OrbitRad * BlendOrbit) + (-DirPE) * FMath::Sin(OrbitRad * BlendOrbit)).GetSafeNormal();

    const FVector CamLoc = Pivot + (-DirPE * Dist) + (SideRot * SideAmt) + FVector(0,0,HeightOffset);

    CounterCam->SetActorLocation(CamLoc);
    CounterCam->SetActorRotation((E - CamLoc).Rotation());

    if (UCineCameraComponent* Cine = CounterCam->GetCineCameraComponent())
    {
       
        Cine->SetFieldOfView(FOV);
    }

    return;
}
*/
    
    
    if (!IsPlayerTurn() && InputMode == EBattleInputMode::FreeAim)
    {
        OnFreeAimEnd(); // 내부에서 InputMode None, Anim->SetFreeAim(false), UI 정리까지 함
    }
    if (InputMode != EBattleInputMode::FreeAim) return;
    if (!IsPlayerTurn()) return;
    float DX = 0.f, DY = 0.f;
    GetInputMouseDelta(DX, DY);

    // 데드존(선택)
    if (FMath::Abs(DX) < AimDeadZone) DX = 0.f;
    if (FMath::Abs(DY) < AimDeadZone) DY = 0.f;

    // 반전 적용
    const float SignX = bInvertX ? -1.f : 1.f;
    const float SignY = bInvertY ?  1.f : -1.f;
    
    UpdateAimTarget();

    const float YawDelta   = DX * AimYawSensitivity * SignX;
    const float PitchDelta = DY * AimPitchSensitivity * SignY;

    if (ABattleCameraActor* Cam = GetBattleCam())
    {
        Cam->AddAimInput(YawDelta, PitchDelta);
    }
    
}

bool ABattlePlayerController::TraceFromCameraCenter(FHitResult& OutHit) const
{
    
    
    FVector Start;
    FRotator Rot;

    if (ABattleCameraActor* CamActor = Cast<ABattleCameraActor>(GetViewTarget()))
    {
        if (UCameraComponent* CamComp = CamActor->GetCameraComponent())
        {
            Start = CamComp->GetComponentLocation();
            Rot   = CamComp->GetComponentRotation();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[PC] Trace: CamComp null"));
            return false;
        }
    }
    else if (PlayerCameraManager)
    {
        Start = PlayerCameraManager->GetCameraLocation();
        Rot   = PlayerCameraManager->GetCameraRotation();
    }
    else
    {
        return false;
    }

    const FVector End = Start + Rot.Vector() * AimTraceDistance;

    FCollisionQueryParams Params(SCENE_QUERY_STAT(FreeAimTrace), true);
    Params.bReturnPhysicalMaterial = false;

    // 자기 자신/카메라 무시
    if (ABattleUnitActor* Me = GetCurrentUnit()) Params.AddIgnoredActor(Me);
    Params.AddIgnoredActor(GetViewTarget());

    // 필요하면 Debug
    // DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 0.02f, 0, 1.f);
    const float Radius = 8.f;
    return GetWorld()->SweepSingleByChannel(
       OutHit,
       Start,
       End,
       FQuat::Identity,
       ECC_Visibility,
       FCollisionShape::MakeSphere(Radius),
       Params
   );
    
}

void ABattlePlayerController::UpdateAimTarget()
{
    FHitResult Hit;
    if (!TraceFromCameraCenter(Hit))
    {
        ClearAimHighlight();
        return;
    }

    ABattleUnitActor* HitUnit = Cast<ABattleUnitActor>(Hit.GetActor());
    if (!HitUnit || !HitUnit->IsEnemyUnit() || !HitUnit->IsAlive())
    {
        ClearAimHighlight();
        return;
    }

    if (AimedUnit.Get() != HitUnit)
    {
        if (AimedUnit.IsValid())
            AimedUnit->SetSelected(false);

        AimedUnit = HitUnit;
        AimedUnit->SetSelected(true);
    }

    LastAimHit = Hit;
}

void ABattlePlayerController::ClearAimHighlight()
{
    if (AimedUnit.IsValid())
        AimedUnit->SetSelected(false);

    AimedUnit = nullptr;
}

/*
void ABattlePlayerController::StartCounterCamera(ABattleUnitActor* PlayerUnit, ABattleUnitActor* EnemyUnit)
{
    if (!GetWorld() || !BattleCameraActor || !PlayerUnit || !EnemyUnit) return;

    SavedViewTarget = GetViewTarget();

    CounterPlayer = PlayerUnit;
    CounterEnemy  = EnemyUnit;

    CounterFromLoc = BattleCameraActor->GetActorLocation();
    CounterFromRot = BattleCameraActor->GetActorRotation();

    bCounterCamActive = true;
    bCounterCamSettled = false;
    CounterStartRealTime = GetWorld()->GetRealTimeSeconds();

    // 뷰 타겟은 "배틀 카메라"로 유지, 대신 카메라 액터 자체를 움직인다
    SetViewTargetWithBlend(BattleCameraActor, CounterBlendIn, EViewTargetBlendFunction::VTBlend_Cubic);
}
*/

void ABattlePlayerController::EndCounterCamera(bool bBlendBack)
{
    bCounterCamActive = false;
    CounterElapsed = 0.f;
    bImpactTriggered = false;
    bCounterCamSettled = false;
    
    /*AActor* Target = CounterReturnTarget ? CounterReturnTarget : (AActor*)GetPawn();

    if (bBlendBack && Target)
    {
        SetViewTargetWithBlend(Target, BlendOutTime);
    }
    else if (Target)
    {
        SetViewTarget(Target);
    }

    UE_LOG(LogTemp, Warning, TEXT("[PC] CounterCam End | BackTo=%s"), *GetNameSafe(Target));
    */
    
    if (GetWorld())
    {
        UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
    }
    if (BattleCam)
    {
        BattleCam->SetCinematicOverride(false);
    }
    if (SavedViewTarget)
    {
        SetViewTargetWithBlend(SavedViewTarget, bBlendBack ? CounterBlendOut : 0.f, EViewTargetBlendFunction::VTBlend_Cubic,2.f);
    }
    
    CounterReturnTarget = nullptr;
    CounterPlayer = nullptr;
    CounterEnemy = nullptr;
    BattleCam = nullptr;
    
}

void ABattlePlayerController::StartCounterCamera(class ABattleUnitActor* PlayerUnit, class ABattleUnitActor* EnemyUnit)
{
    UE_LOG(LogTemp, Warning, TEXT("[Counter] Start | ViewTarget=%s | BattleCameraActor=%s | BattleCam=%s"),
    *GetNameSafe(GetViewTarget()),
    *GetNameSafe(BattleCameraActor),
    *GetNameSafe(BattleCam));
    if (!GetWorld() || !BattleCameraActor || !PlayerUnit || !EnemyUnit) return;

    SavedViewTarget = GetViewTarget();
    ABattleCameraActor* Cam = Cast<ABattleCameraActor>(GetViewTarget());
    if (!Cam)
    {
        Cam = GetBattleCam();
    }
    if (!Cam)
    {
        UE_LOG(LogTemp , Warning , TEXT("NO COUNTER"));
        return;
    }
    
    if (InputMode == EBattleInputMode::FreeAim)
    {
        OnFreeAimEnd();
    }
    
   
    BattleCam = Cam;
    BattleCameraActor = Cam;
    
    CounterPlayer = PlayerUnit;
    CounterEnemy  = EnemyUnit;
    
    bCounterCamActive = true;
    bImpactTriggered = false;
    CounterElapsed = 0.f;
    
    
    CounterFromLoc = BattleCameraActor->GetActorLocation();
    CounterFromRot = BattleCameraActor->GetActorRotation();

    bCounterCamActive = true;
    bCounterCamSettled = false;
   // CounterStartRealTime = GetWorld()->GetRealTimeSeconds();

    BattleCameraActor->SetCinematicOverride(true);
    
    Cam->SetCinematicOverride(true);
    // 뷰 타겟은 "배틀 카메라"로 유지, 대신 카메라 액터 자체를 움직인다
    SetViewTargetWithBlend(Cam,
        CounterBlendIn,
        EViewTargetBlendFunction::VTBlend_Cubic ,
        2.0f);
    UE_LOG(LogTemp, Warning, TEXT("[Counter] Start | Cam=%s"), *GetNameSafe(Cam));
}

void ABattlePlayerController::UpdateCounterCam()
{
    if (!bCounterCamActive || !GetWorld() || !BattleCameraActor) return;
    if (!CounterPlayer.IsValid() || !CounterEnemy.IsValid()) return;

    const FVector P = CounterPlayer->GetActorLocation();
    const FVector E = CounterEnemy->GetActorLocation();

    // LookAt은 Mid로 (원작은 둘 중간을 크게 잡고 위로 살짝)
    FVector LookAt = (P + E) * 0.5f;
    LookAt.Z += CounterLookAtZ;

    // 카메라 구도: "플레이어 뒤쪽 + 왼쪽"에서 둘을 바라봄
    FVector DirPE = (E - P);
    DirPE.Z = 0.f;
    if (DirPE.IsNearlyZero()) DirPE = CounterPlayer->GetActorForwardVector();
    DirPE = DirPE.GetSafeNormal();

    const FVector Right = FVector::CrossProduct(FVector::UpVector, DirPE).GetSafeNormal();

    // 뒤로 물러나면서(side는 왼쪽)
    const FVector TargetLoc =
        LookAt
        - DirPE * CounterCamDistance
        + Right * CounterCamSide
        + FVector(0,0,CounterCamHeight);

    const FRotator TargetRot = (LookAt - TargetLoc).Rotation();

    // "자리잡기"만 짧게 보간 후, 이후엔 거의 고정(트래킹만) 느낌
    float t = (GetWorld()->GetRealTimeSeconds() - CounterStartRealTime) / FMath::Max(0.01f, CounterMoveDuration);
    t = FMath::Clamp(t, 0.f, 1.f);

    if (t < 1.f)
    {
        const float a = EaseOutCubic(t);
        BattleCameraActor->SetActorLocation(FMath::Lerp(CounterFromLoc, TargetLoc, a));
        BattleCameraActor->SetActorRotation(FMath::Lerp(CounterFromRot, TargetRot, a));
    }
    else
    {
        bCounterCamSettled = true;
        // 이후엔 위치는 고정(원작 느낌), 회전만 LookAt으로 유지
        BattleCameraActor->SetActorRotation(TargetRot);
        // (원하면 위치도 아주 약하게 따라가게 하려면 여기서 Lerp 0.1 정도)
    }
    
}

void ABattlePlayerController::TriggerCounterImpact()
{
    if (!GetWorld()) return;

    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), CounterImpactDilation);

    // RealTime 기준 복구
    const float RestoreAt = GetWorld()->GetRealTimeSeconds() + CounterImpactSlowReal;

    // Tick 기반 복구가 가장 안전하지만, 타이머로 할거면 RealTime 기반이 필요함.
    // 간단하게: 다음 틱부터 RestoreAt 체크하는 방식 추천.
    // 여기선 람다 타이머 대신: 멤버 변수로 RestoreAt 저장해서 Tick에서 복구하게 구현해도 됨.
    FTimerHandle TH;
    GetWorld()->GetTimerManager().SetTimer(
        TH,
        FTimerDelegate::CreateLambda([this]()
        {
            if (GetWorld()) UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
        }),
        CounterImpactSlowReal,
        false
    );

    // 카메라 쉐이크(선택)
}

