// Fill out your copyright notice in the Description page of Project Settings.


#include "Battle/BattleCameraActor.h"

#include "BattleUnitActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
ABattleCameraActor::ABattleCameraActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bDoCollisionTest = false;
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->SetRelativeRotation(FRotator(-20.f, 0.f, 0.f));

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
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

