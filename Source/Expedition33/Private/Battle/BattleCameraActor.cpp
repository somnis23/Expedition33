// Fill out your copyright notice in the Description page of Project Settings.


#include "Battle/BattleCameraActor.h"

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
	
	SpringArm->TargetArmLength = 0.f;
	SpringArm->bDoCollisionTest = false;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 7.0f;
	SpringArm->bUsePawnControlRotation = false;
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;
	
}

void ABattleCameraActor::SetupForCenter(const FVector& Center)
{
	const FVector NewLoc = Center + CameraOffset;
	SetActorLocation(NewLoc);
	
	const FRotator LookRot = (Center - NewLoc).Rotation();
	SetActorRotation(LookRot);
	
}

// Called when the game starts or when spawned
void ABattleCameraActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABattleCameraActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

