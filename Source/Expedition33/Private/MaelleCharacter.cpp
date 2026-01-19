// Fill out your copyright notice in the Description page of Project Settings.


#include "MaelleCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
AMaelleCharacter::AMaelleCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	//캡슐 컴포넌트
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	//trans form
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	//카메라
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 350.0f;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 12.f;
	SpringArm->SocketOffset = FVector(0.0f, 45.f, 18.f);
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;
	
	//무브먼트
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->bOrientRotationToMovement = true;
	MoveComp->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	MoveComp->MaxWalkSpeed = WalkSpeed;
	MoveComp->MaxAcceleration = 1200.f;				//가속
	MoveComp->GroundFriction = 6.0f;			//마찰
	MoveComp->BrakingDecelerationWalking = 800.f; // 감속
	
	MoveComp->AirControl = 0.3f;
	
	Speed = 0.f;
	bIsInAir = false;
	bIsSprint = false;
	
}

// Called when the game starts or when spawned
void AMaelleCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	
	if (APlayerController* PlayerCon = Cast<APlayerController>( Controller)){
			if (ULocalPlayer* LocalPlayer = PlayerCon->GetLocalPlayer())
    		{
    			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
    			{
    				Subsystem->AddMappingContext(DefaultMappingContext,0);
    			}
    		}
	}
}

// Called every frame
void AMaelleCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	const float TargetSpeed = GetVelocity().Size2D();

	// ✅ 핵심: 보간
	SmoothedSpeed = FMath::FInterpTo(
		SmoothedSpeed,
		TargetSpeed,
		DeltaTime,
		8.f   // 보간 속도 
	);

	Speed = SmoothedSpeed;
	bIsInAir = GetCharacterMovement()->IsFalling();
	
	
}

// Called to bind functionality to input
void AMaelleCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UEnhancedInputComponent* Input = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	Input->BindAction(MoveAction , ETriggerEvent::Triggered , this , &AMaelleCharacter::Move);
	Input->BindAction(LookAction , ETriggerEvent::Triggered , this , &AMaelleCharacter::Look);

	if (SprintAction)
	{
		Input->BindAction(SprintAction , ETriggerEvent::Started , this , &AMaelleCharacter::SprintStart);
		Input->BindAction(SprintAction , ETriggerEvent::Completed , this , &AMaelleCharacter::SprintStop);
		
		Input->BindAction(SprintAction, ETriggerEvent::Canceled, this, &AMaelleCharacter::SprintStop);
		
	}
	

}

void AMaelleCharacter::Move(const FInputActionValue& Value)
{
	FVector2D Axis = Value.Get<FVector2D>();
	
	if (Controller)
	{
		const FRotator YawRot(0.f , Controller->GetControlRotation().Yaw , 0.0f);
		
		const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
		const FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
		
		AddMovementInput(Forward , Axis.Y);
		AddMovementInput(Right , Axis.X);
		
		
		
	}
	
}

void AMaelleCharacter::Look(const FInputActionValue& Value)
{
	FVector2D Axis = Value.Get<FVector2D>();
	
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
	
	
}

void AMaelleCharacter::SprintStart()
{
	bIsSprint =true;
	auto* Move = GetCharacterMovement();
	Move->MaxWalkSpeed = SprintSpeed;
	Move->BrakingDecelerationWalking = 600.f;
	
	UE_LOG(LogTemp, Warning, TEXT("Sprint Speed: %f"), SprintSpeed);
	
}

void AMaelleCharacter::SprintStop()
{
	bIsSprint = false;
	auto* Move = GetCharacterMovement();
	Move->MaxWalkSpeed = WalkSpeed;
	Move->BrakingDecelerationWalking = 800.f;
}

