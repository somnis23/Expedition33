// Fill out your copyright notice in the Description page of Project Settings.


#include "Battle/BattleCameraActor.h"

#include "BattleUnitActor.h"
#include "DetailCategoryBuilder.h"
#include "Camera/CameraComponent.h"

#include "GameFramework/SpringArmComponent.h"

// Sets default values
ABattleCameraActor::ABattleCameraActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bDoCollisionTest = false;
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->SetRelativeRotation(FRotator(-20.f, 0.f, 0.f));

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	
	TargetFOV = DefaultFOV;
	TargetArmLength = DefaultArm;
	TargetOffset = DefaultOffset;
}

void ABattleCameraActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bCinematicOverride)
	{
		if (CineType == ECinematicType::Phantom)
		{
			UpdatePhantomStrikeCam(DeltaSeconds);
		} 
		else if (CineType == ECinematicType::Gommage)
		{
			UpdateGommageCam(DeltaSeconds);
		}
		
		
		Camera->FieldOfView = FMath::FInterpTo(Camera->FieldOfView, TargetFOV, DeltaSeconds, BlendSpeed);
		SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, TargetArmLength, DeltaSeconds, BlendSpeed);
		SpringArm->SocketOffset = FMath::VInterpTo(SpringArm->SocketOffset, TargetOffset, DeltaSeconds, BlendSpeed);

		SetActorRotation(FRotator(0.f, CurrentYaw, 0.f));
		SpringArm->SetRelativeRotation(FRotator(CurrentPitch, 0.f, 0.f));
		return;
	}

	if (Anchor.IsValid())
	{
		const float PivotZ = (CamMode == EBattleCamMode::FreeAim) ? FreeAimPivotZ : BattlePivotZ;
		SetActorLocation(Anchor->GetActorLocation() + FVector(0,0,PivotZ));
	}
	
	switch (CamMode)
	{
	case EBattleCamMode::FreeAim:
		UpdateFreeAimCam(DeltaSeconds);
		break;

	case EBattleCamMode::TargetSelect:
		UpdateTargetSelectCam(DeltaSeconds);
		break;

	default:
		UpdateBattleCam(DeltaSeconds);
		break;
	}
	
	/*if (Anchor.IsValid())
	{
		SetActorLocation(Anchor->GetActorLocation());
	}*/
	Camera->FieldOfView = FMath::FInterpTo(Camera->FieldOfView, TargetFOV, DeltaSeconds, BlendSpeed);
	SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, TargetArmLength, DeltaSeconds, BlendSpeed);
	SpringArm->SocketOffset = FMath::VInterpTo(SpringArm->SocketOffset, TargetOffset, DeltaSeconds, BlendSpeed);
	
	SetActorRotation(FRotator(0.f, CurrentYaw, 0.f));
	SpringArm->SetRelativeRotation(FRotator(CurrentPitch, 0.f, 0.f));
}

void ABattleCameraActor::UpdateCameraTransform()
{
	const FVector PlayerLoc = Player->GetActorLocation();
	const FVector EnemyLoc  = Enemy->GetActorLocation();

	// 전투 중심
	const FVector Center = (PlayerLoc + EnemyLoc) * 0.5f;
	SetActorLocation(Center);

	// 거리 계산
	float Distance = FVector::Distance(PlayerLoc, EnemyLoc);
	Distance = FMath::Clamp(
		Distance * DistanceMultiplier,
		MinDistance,
		MaxDistance
	);

	SpringArm->TargetArmLength = Distance;
	SpringArm->SocketOffset = FVector(-300.f, 0.f, Height);

	// 방향 (플레이어 → 적 기준)
	const FVector LookDir = (EnemyLoc - PlayerLoc).GetSafeNormal();
	const FRotator YawRot = LookDir.Rotation();

	SetActorRotation(FRotator(0.f, YawRot.Yaw, 0.f));
}
static float EaseOutCubic(float x)
{
	 x= FMath::Clamp(x , 0.f , 1.f);
	return 1.f - FMath::Pow(1.f - x, 3.f);
}

void ABattleCameraActor::UpdatePhantomStrikeCam(float DT)
{
	if (!PhantomAttacker.IsValid() || !PhantomTarget.IsValid())
    {
        // 대상이 날아갔으면 안전 복귀
        bCinematicOverride = false;
		CineType = ECinematicType::None;
        PhantomBeat = EPhantomCamBeat::None;
        return;
    }

    PhantomBeatTime += DT;
    const float A = PhantomBeatTime / PhantomBeatDuration;
    const float E = EaseOutCubic(A);

    const FVector AttLoc = PhantomAttacker->GetActorLocation();
    const FVector TarLoc = PhantomTarget->GetActorLocation();

    // 카메라 피벗(ActorLocation)은 “둘 사이”를 기반으로 잡는 게 안정적
    const FVector Center = FMath::Lerp(AttLoc, TarLoc, 0.55f);
    SetActorLocation(Center + FVector(0,0,BattlePivotZ));

    // 방향(공격자 -> 타겟)
    const FVector LookDir = (TarLoc - AttLoc).GetSafeNormal();
    const float DesiredYaw = LookDir.Rotation().Yaw;

    if (PhantomBeat == EPhantomCamBeat::Start)
    {
        // Start: 살짝 줌인 + 오프셋 옆으로 + Pitch 조금 낮게
        TargetFOV       = FMath::Lerp(PhantomStartFOV, PhantomStartFOV - 10.f, E);
        TargetArmLength = FMath::Lerp(PhantomStartArm, DefaultArm * 0.75f, E);
        TargetOffset    = FMath::Lerp(PhantomStartOffset, FVector(-240.f, 80.f, Height + 35.f), E);

        CurrentYaw   = FMath::FInterpTo(CurrentYaw, DesiredYaw, DT, 10.f);
        CurrentPitch = FMath::FInterpTo(CurrentPitch, -16.f, DT, 10.f);

        if (A >= 1.f)
        {
            // Dash로 자동 전환(여기선 “유지”에 가깝게)
            PhantomGotoBeat(EPhantomCamBeat::Dash, 0.35f);
        }
    }
    else if (PhantomBeat == EPhantomCamBeat::Dash)
    {
        
        TargetFOV       = FMath::FInterpTo(TargetFOV, DefaultFOV + 6.f, DT, 6.f);
        TargetArmLength = FMath::FInterpTo(TargetArmLength, DefaultArm * 0.70f, DT, 6.f);
        TargetOffset    = FMath::VInterpTo(TargetOffset, FVector(-200.f, 30.f, Height + 30.f), DT, 6.f);

        CurrentYaw   = FMath::FInterpTo(CurrentYaw, DesiredYaw, DT, 14.f);
        CurrentPitch = FMath::FInterpTo(CurrentPitch, -14.f, DT, 10.f);

        
    }
    else if (PhantomBeat == EPhantomCamBeat::Impact)
    {
    	TargetFOV       = FMath::FInterpTo(TargetFOV, DefaultFOV + 6.f, DT, 6.f);
    	TargetArmLength = FMath::FInterpTo(TargetArmLength, DefaultArm * 0.70f, DT, 6.f);
    	TargetOffset    = FMath::VInterpTo(TargetOffset, FVector(-200.f, 30.f, Height + 30.f), DT, 6.f);

    	CurrentYaw   = FMath::FInterpTo(CurrentYaw, DesiredYaw, DT, 14.f);
    	CurrentPitch = FMath::FInterpTo(CurrentPitch, -14.f, DT, 10.f);
    	
    	
    	
        // Impact: 강한 줌인 + 타겟에 더 붙이기 (짧게)
        TargetFOV       = FMath::FInterpTo(TargetFOV, DefaultFOV - 14.f, DT, 18.f);
        TargetArmLength = FMath::FInterpTo(TargetArmLength, DefaultArm * 0.62f, DT, 18.f);
        TargetOffset    = FMath::VInterpTo(TargetOffset, FVector(-170.f, 10.f, Height + 45.f), DT, 18.f);

        CurrentYaw   = FMath::FInterpTo(CurrentYaw, DesiredYaw, DT, 18.f);
        CurrentPitch = FMath::FInterpTo(CurrentPitch, -12.f, DT, 18.f);

        
        if (A >= 1.f)
        {
            PhantomGotoBeat(EPhantomCamBeat::Dash, 0.25f);
        }
    }
    else if (PhantomBeat == EPhantomCamBeat::End)
    {
        
        bCinematicOverride = false;
    	CineType = ECinematicType::None;
        PhantomBeat = EPhantomCamBeat::None;
    }

}

void ABattleCameraActor::PhantomGotoBeat(EPhantomCamBeat NextBeat, float Duration)
{
	PhantomBeat = NextBeat;
	PhantomBeatTime = 0.f;
	PhantomBeatDuration = FMath::Max(Duration, 0.01f);
	
}

void ABattleCameraActor::StartPhantomStrikeCam(ABattleUnitActor* Attacker, ABattleUnitActor* Target)
{
	if (!Attacker || !Target) return;

	PhantomAttacker = Attacker;
	PhantomTarget   = Target;

	
	bCinematicOverride = true;
	CineType = ECinematicType::Phantom;
	// 시작 스냅(현재 카메라 상태를 시작점으로)
	PhantomStartYaw   = CurrentYaw;
	PhantomStartPitch = CurrentPitch;
	PhantomStartFOV   = Camera->FieldOfView;
	PhantomStartArm   = SpringArm->TargetArmLength;
	PhantomStartOffset= SpringArm->SocketOffset;

	PhantomGotoBeat(EPhantomCamBeat::Start, 0.18f);
}

void ABattleCameraActor::PhantomStrikeImpactBeat()
{
	if (!bCinematicOverride) return;
	if (!PhantomAttacker.IsValid()|| !PhantomTarget.IsValid()) return;
	
	PhantomGotoBeat(EPhantomCamBeat::Impact , 0.12f);
	
	
}

void ABattleCameraActor::EndPhantomStrikeCam()
{
	if (!bCinematicOverride) return;
	PhantomGotoBeat(EPhantomCamBeat::End , 0.22f);
	
}

void ABattleCameraActor::StartGommageCam(ABattleUnitActor* Caster, ABattleUnitActor* Target)
{
	if (!Caster || !Target) return;

	bCinematicOverride = true;
	CineType = ECinematicType::Gommage;

	GommageCaster = Caster;
	GommageTarget = Target;

	GommageBeat = EGommageBeat::Start;
	GommageT = 0.f;
	GommageDur = 0.18f;
	
}

void ABattleCameraActor::GommageFocusBeat()
{
	
	
}

void ABattleCameraActor::GommageImpactBeat()
{
	if (!bCinematicOverride || CineType != ECinematicType::Gommage) return;

	GommageBeat = EGommageBeat::Impact;
	GommageT = 0.f;
	GommageDur = 0.10f; // 짧게 펄스
}

void ABattleCameraActor::EndGommageCam()
{
	if (!bCinematicOverride || CineType != ECinematicType::Gommage) return;

	GommageBeat = EGommageBeat::End;
	GommageT = 0.f;
	GommageDur = 0.22f;
}

void ABattleCameraActor::UpdateGommageCam(float DT)
{
	if (!GommageCaster.IsValid() || !GommageTarget.IsValid())
    {
        bCinematicOverride = false;
        CineType = ECinematicType::None;
        GommageBeat = EGommageBeat::None;
        return;
    }

    GommageT += DT;
    float A = GommageT / FMath::Max(GommageDur, 0.01f);
    float E = 1.f - FMath::Pow(1.f - FMath::Clamp(A,0.f,1.f), 3.f);

    const FVector C = GommageCaster->GetActorLocation();
    const FVector T = GommageTarget->GetActorLocation();

   
    FVector Center = FMath::Lerp(C, T, 0.90f);
    SetActorLocation(Center + FVector(0,0,BattlePivotZ));

    const FVector LookDir = (T - C).GetSafeNormal();
    float DesiredYaw = LookDir.Rotation().Yaw;

    if (GommageBeat == EGommageBeat::Start)
    {
    	// 1) 피벗을 캐스터에 고정 (영상 5초 느낌 핵심)
    	SetActorLocation(C + FVector(0,0,BattlePivotZ));

    	// 2) 캐릭터 정면을 보게: caster yaw + 180 (+side)
    	const float CasterYaw = GommageCaster->GetActorRotation().Yaw;
    	DesiredYaw = CasterYaw + 180.f + 12.f;

    	// 3) 쇼케이스 구도(캐릭터 크게)
    	TargetFOV       = FMath::Lerp(DefaultFOV, 38.f, E);
    	TargetArmLength = FMath::Lerp(DefaultArm, DefaultArm * 0.55f, E);
    	TargetOffset    = FMath::Lerp(DefaultOffset, FVector(+120.f, 0.f, Height + 35.f), E);

    	SetActorLocation(C + FVector(0,0,BattlePivotZ));
    	const FVector CamPivot = GetActorLocation();
    	const FRotator Look = (C + FVector(0,0,90.f) - CamPivot).Rotation();
    	CurrentYaw   = FMath::FInterpTo(CurrentYaw, Look.Yaw, DT, 10.f);
    	CurrentPitch = FMath::FInterpTo(CurrentPitch, -10.f, DT, 10.f);
    	
    	
    	/*CurrentYaw   = FMath::FInterpTo(CurrentYaw, DesiredYaw, DT, 10.f);
    	CurrentPitch = FMath::FInterpTo(CurrentPitch, -10.f, DT, 10.f);*/

    	if (A >= 1.f)
    	{
    		
    		GommageBeat = EGommageBeat::Hold;
    		GommageT = 0.f;
    		GommageDur = 999.f;
    	}
    }
    else if (GommageBeat == EGommageBeat::Hold)
    {
        // 유지 중에도 타겟을 계속 바라보게만
    	SetActorLocation(C + FVector(0,0,BattlePivotZ));

    	const float CasterYaw = GommageCaster->GetActorRotation().Yaw;
    	DesiredYaw = CasterYaw + 180.f + 12.f;

    	TargetFOV       = FMath::Lerp(DefaultFOV, 38.f, E);
    	TargetArmLength = FMath::Lerp(DefaultArm, DefaultArm * 0.55f, E);
    	TargetOffset    = FMath::Lerp(DefaultOffset, FVector(+120.f, 0.f, Height + 35.f), E);

    	SetActorLocation(C + FVector(0,0,BattlePivotZ));
    	const FVector CamPivot = GetActorLocation();
    	const FRotator Look = (C + FVector(0,0,90.f) - CamPivot).Rotation();
    	CurrentYaw   = FMath::FInterpTo(CurrentYaw, Look.Yaw, DT, 10.f);
    	CurrentPitch = FMath::FInterpTo(CurrentPitch, -10.f, DT, 10.f);

    	/*CurrentYaw   = FMath::FInterpTo(CurrentYaw, DesiredYaw, DT, 10.f);
    	CurrentPitch = FMath::FInterpTo(CurrentPitch, -10.f, DT, 10.f);*/
    }
    else if (GommageBeat == EGommageBeat::Impact)
    {
    	Center = FMath::Lerp(C, T, 0.62f);
    	SetActorLocation(Center + FVector(0,0,BattlePivotZ));

    	const FVector ToTarget = (T + FVector(0,0,90.f)) - GetActorLocation();
    	DesiredYaw = ToTarget.Rotation().Yaw;
    	const float DesiredPitch = FMath::Clamp(ToTarget.Rotation().Pitch, PitchMin, PitchMax);

    	TargetFOV       = FMath::FInterpTo(TargetFOV, 32.f, DT, 20.f);             // 더 줌
    	TargetArmLength = FMath::FInterpTo(TargetArmLength, DefaultArm * 0.50f, DT, 20.f);
    	TargetOffset    = FMath::VInterpTo(TargetOffset, FVector(-170.f, 20.f, Height + 45.f), DT, 20.f);

    	CurrentYaw   = FMath::FInterpTo(CurrentYaw, DesiredYaw, DT, 18.f);
    	CurrentPitch = FMath::FInterpTo(CurrentPitch, DesiredPitch, DT, 18.f);

    	if (A >= 1.f)
    	{
    		// 다시 쇼케이스 유지로 복귀(영상도 보통 다시 캐스터 컷으로 돌아감)
    		GommageBeat = EGommageBeat::Hold;
    		GommageT = 0.f;
    		GommageDur = 999.f;
    	}
    }
    else if (GommageBeat == EGommageBeat::End)
    {
        // 복귀: 시네마 해제하면 기존 UpdateBattleCam이 자연 복귀시킴
        bCinematicOverride = false;
        CineType = ECinematicType::None;
        GommageBeat = EGommageBeat::None;
    }
}

void ABattleCameraActor::BeginPlay()
{
	Super::BeginPlay();
}

void ABattleCameraActor::Init(ABattleUnitActor* InPlayer, ABattleUnitActor* InEnemy)
{
	Player = InPlayer;
	Enemy = InEnemy;
	
	if (!Player || !Enemy)
	{
		UE_LOG(LogTemp, Error, TEXT("BattleCamera Init failed: Player or Enemy null"));
		return;
		
	}
	
	UpdateCameraTransform();
	
}

void ABattleCameraActor::SetTargetSelect(bool bEnable, ABattleUnitActor* InTarget)
{
	if (CamMode == EBattleCamMode::FreeAim) return;
	
	CamMode = bEnable ? EBattleCamMode::TargetSelect : EBattleCamMode::Battle;
	FocusTarget = InTarget;
	
	if (bEnable)
	{
		TargetFOV = 45.f;
		TargetArmLength = DefaultArm * 0.85f;
		TargetOffset = FVector(-260.f,40.f,Height+20.f);
	}
	else
	{
		TargetFOV = DefaultFOV;
		TargetArmLength = DefaultArm;
		TargetOffset = DefaultOffset;
	}
	
	
}


void ABattleCameraActor::SetFreeAim(bool bEnable)
{
	UE_LOG(LogTemp, Warning, TEXT("[Cam] SetFreeAim=%s"), bEnable ? TEXT("true") : TEXT("false"));

	// 같은 값이면 불필요 갱신 방지
	if (bFreeAim == bEnable) return;

	bFreeAim = bEnable;

	
	CamMode = bFreeAim ? EBattleCamMode::FreeAim : EBattleCamMode::Battle;

	
	FocusTarget = nullptr;

	bFreeAimJustEntered = bFreeAim; // 진입 프레임에만 스냅

	if (bFreeAim)
	{
		TargetFOV       = AimFOV;
		TargetArmLength = AimArm;
		TargetOffset    = AimOffset;
	}
	else
	{
		TargetFOV       = DefaultFOV;
		TargetArmLength = DefaultArm;
		TargetOffset    = DefaultOffset;
	}
	
}

void ABattleCameraActor::AddAimInput(float YawDelta, float PitchDelta)
{
	if (CamMode != EBattleCamMode::FreeAim) return;

	CurrentYaw += YawDelta;
	CurrentPitch = FMath::Clamp(CurrentPitch+PitchDelta , PitchMin, PitchMax);

}

void ABattleCameraActor::UpdateBattleCam(float DT)
{
	if (!Player || !Enemy) return;

	const FVector PlayerLoc = Player->GetActorLocation();
	const FVector EnemyLoc  = Enemy->GetActorLocation();

	// 전투 중심
	const FVector Center = (PlayerLoc + EnemyLoc) * 0.5f;
	SetActorLocation(Center);

	// 거리 기반 암 길이
	float Distance = FVector::Distance(PlayerLoc, EnemyLoc);
	Distance = FMath::Clamp(Distance * DistanceMultiplier, MinDistance, MaxDistance);

	//  배틀 모드 목표값
	TargetFOV = DefaultFOV;
	TargetArmLength = Distance;
	TargetOffset = FVector(-300.f, 0.f, Height);

	//  배틀 모드 yaw: 플레이어 → 적
	const FVector LookDir = (EnemyLoc - PlayerLoc).GetSafeNormal();
	const float DesiredYaw = LookDir.Rotation().Yaw;

	CurrentYaw = FMath::FInterpTo(CurrentYaw, DesiredYaw, DT, 8.f);
	CurrentPitch = -20.f; 
	
}

void ABattleCameraActor::UpdateFreeAimCam(float DT)
{
	if (!bFreeAim) return;

	if (bFreeAimJustEntered)
	{
		bFreeAimJustEntered = false;

		if (Player && Enemy)
		{
			const FVector P = Player->GetActorLocation() + FVector(0,0,110.f);
			const FVector E = Enemy->GetActorLocation() + FVector(0,0,90.f);
			const FRotator Look = (E - P).Rotation();

			CurrentYaw   = Look.Yaw;
			CurrentPitch = FMath::Clamp(Look.Pitch, PitchMin, PitchMax);
		}
	}
	
}

void ABattleCameraActor::UpdateTargetSelectCam(float DT)
{
	if (!Player || !Enemy) return;
	
	const FVector PlayerLoc = Player->GetActorLocation();
	const FVector EnemyLoc = Enemy->GetActorLocation();
	
	FVector FocusLoc = EnemyLoc;
	
	if (FocusTarget.IsValid())
	{
		FocusLoc = FocusTarget->GetActorLocation();
		
	}
	
	// 중심을 플레이어 > 타겟 쪽으로 당김
	const FVector Center = FMath::Lerp(PlayerLoc , FocusLoc,0.6f);
	SetActorLocation(Center);
	
	const FVector LookDir = (FocusLoc - PlayerLoc).GetSafeNormal();
	const float TargetYaw = LookDir.Rotation().Yaw;
	
	CurrentYaw = FMath::FInterpTo(CurrentYaw, TargetYaw, DT, 8.f);
	
	const float DesiredPitch  =  -18.f;
	CurrentPitch = FMath::FInterpTo(CurrentPitch, DesiredPitch, DT, 6.f);
	
	
}



