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
    
    /*// FreeAim 중이 아니거나 내 턴 아니면 아무것도 안 함
    if (InputMode != EBattleInputMode::FreeAim) return;
    if (!IsPlayerTurn()) return;
    
    if (InputMode == EBattleInputMode::FreeAim && IsPlayerTurn())
    {
        UpdateAimTarget();
        
        float DX, DY;
        GetInputMouseDelta(DX, DY);
        
        if (ABattleCameraActor* Cam = GetBattleCam())
        {
            Cam->AddAimInput(DX * 0.8f, -DY * 0.8f); // 감도
        }
    }*/
    
    

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

