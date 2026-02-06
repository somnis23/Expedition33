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
	
	
	
	
	
	
	
	/////////////////////////////////////////////////////////////////////////////////
	///
	///
	///
	///
	/*
	 *
	UE_LOG(LogTemp, Warning, TEXT("[Cam] SetFreeAim=%s"), bEnable ? TEXT("true") : TEXT("false"));
	bFreeAim = bEnable;
	CamMode  = bEnable ? EBattleCamMode::FreeAim : EBattleCamMode::Battle; 
	
	
	if (bFreeAim)
	{
		
		if (Player && Enemy)
		{
			const FVector P = Player->GetActorLocation() + FVector(0,0,110.f);
			const FVector E = Enemy->GetActorLocation() + FVector(0,0,90.f);
			const FRotator Look = (E - P).Rotation();

			CurrentYaw = Look.Yaw;
			CurrentPitch = FMath::Clamp(Look.Pitch, PitchMin, PitchMax);
		}
		// FreeAim 진입 시 기본 회전값 유지(튐 방지)
		// 필요하면 여기서 CurrentYaw를 현재 ActorYaw로 스냅
		

		TargetFOV = AimFOV;
		TargetArmLength = AimArm;
		TargetOffset = AimOffset;
	}
	else
	{
		TargetFOV = DefaultFOV;
		TargetArmLength = DefaultArm;
		TargetOffset = DefaultOffset;
	}
	*/
	
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



