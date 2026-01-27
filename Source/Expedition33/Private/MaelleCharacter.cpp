// Fill out your copyright notice in the Description page of Project Settings.


#include "MaelleCharacter.h"

#include "EnemyCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ExpeditionGameInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

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


void AMaelleCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	const float TargetSpeed = GetVelocity().Size2D();

	//  핵심: 보간
	SmoothedSpeed = FMath::FInterpTo(
		SmoothedSpeed,
		TargetSpeed,
		DeltaTime,
		8.f   // 보간 속도 
	);
	
	if (bEncounterPlaying)
	{
		UpdateBattleEncounter(DeltaTime);
	}
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

void AMaelleCharacter::DoBattleTravel()
{
	
}
/*{
	UE_LOG(LogTemp, Warning, TEXT("DoBattleTravel"));

	if (!CachedGameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("CachedGameInstance is NULL"));
		return;
	}

	CachedGameInstance->EnterBattle(
		GetWorld()->GetFName(),
		GetActorTransform(),
		PendingBattleEnemyClass
	);

	UE_LOG(LogTemp, Warning, TEXT("OPEN LV_Battle"));
	UGameplayStatics::OpenLevel(GetWorld(), FName("LV_Battle"));
	
}*/

void AMaelleCharacter::StartBattleEncounter(AEnemyCharacter* Enemy)
{
	UE_LOG(LogTemp, Warning, TEXT("StartEncounter"));
	bInEnCounter = true;
	PendingBattleEnemyClass = Enemy->BattleActorClass;
	
	if (CachedGameInstance)
	{
		CachedGameInstance->StopBGM(0.5f);
	}

	// 월드 슬로우(퍼즈 X)
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.1f);

	// 입력 차단
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);

		// 컨트롤 로테이션을 기준으로 카메라를 돌리는 방식
		
		EncounterStartControlRot = PC->GetControlRotation();

		const float YawOffset = 50.f; // 50~70 
		EncounterTargetControlRot = EncounterStartControlRot + FRotator(0.f, YawOffset, 0.f);
	}

	EncounterElapsed = 0.f;
	bEncounterPlaying = true;
}

void AMaelleCharacter::UpdateBattleEncounter(float DeltaTime)
{
	EncounterElapsed += DeltaTime;

	float Alpha = FMath::Clamp(EncounterElapsed / EncounterDuration, 0.f, 1.f);
	float SmoothAlpha = FMath::InterpEaseOut(0.f, 1.f, Alpha, 2.5f);

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		FRotator NewRot = FMath::Lerp(EncounterStartControlRot, EncounterTargetControlRot, SmoothAlpha);
		PC->SetControlRotation(NewRot);
	}

	if (Alpha >= 1.f)
	{
		FinishBattleEncounter();
	}
}

void AMaelleCharacter::FinishBattleEncounter()
{
	UE_LOG(LogTemp, Warning, TEXT("FinishEncounter -> OPEN LV_Battle"));
	bInEnCounter = false;
	bEncounterPlaying = false;

	// 슬로우 복구(중요)
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);

	if (!CachedGameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("CachedGameInstance NULL in FinishEncounter"));
		return;
	}

	// 복귀 레벨명: GetWorld()->GetFName()도 되지만,
	// 실제 맵 이름이 필요하면 GetCurrentLevelName
	const FName ReturnLevel = FName(*UGameplayStatics::GetCurrentLevelName(this, true));

	CachedGameInstance->EnterBattle(
		ReturnLevel,
		GetActorTransform(),
		PendingBattleEnemyClass
	);

	UGameplayStatics::OpenLevel(GetWorld(), FName("LV_Battle"));
}


void AMaelleCharacter::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	
	UE_LOG(LogTemp, Warning, TEXT("NotifyActorBeginOverlap"));

	// 이미 전투 이동 요청이 걸려있으면 중복 방지
	if (bPendingBattle)
	{
		UE_LOG(LogTemp, Warning, TEXT("Battle travel already pending"));
		return;
	}

	AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(OtherActor);
	if (!Enemy) return;
	if (!Enemy->bCanTriggerBattle) return;

	// 중복 진입 방지
	Enemy->bCanTriggerBattle = false;
	
	CachedGameInstance = GetGameInstance<UExpeditionGameInstance>();
	if (!CachedGameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("CachedGameInstance NULL at overlap"));
		return;
	}
	StartBattleEncounter(Enemy);
	/*
	// 전투 대상 클래스 저장
	PendingBattleEnemyClass = Enemy->GetClass();

	// 전투 요청 플래그
	bPendingBattle = true;
	
	UE_LOG(LogTemp, Warning, TEXT("Battle request queued -> scheduling timer"));

	// 타이머: "다음 틱에" DoBattleTravel 실행
	GetWorldTimerManager().SetTimerForNextTick(this, &AMaelleCharacter::DoBattleTravel);
	*/
	
	
	
}


