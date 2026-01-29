// Fill out your copyright notice in the Description page of Project Settings.


#include "Battle/BattlePlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

void ABattlePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	UE_LOG(LogTemp, Error, TEXT("### SetupInputComponent ###"));
	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
	
	if (!EIC) return;
	
	EIC->BindAction(IA_Attack, ETriggerEvent::Started,
		this, &ABattlePlayerController::OnAttackPressed);
	
	EIC->BindAction(IA_Confirm, ETriggerEvent::Started,
		this, &ABattlePlayerController::OnConfirm);
	
	EIC->BindAction(IA_SelectNext, ETriggerEvent::Started,
		this, &ABattlePlayerController::OnSelectNext);
	
	EIC->BindAction(IA_SelectPrev, ETriggerEvent::Started,
		this, &ABattlePlayerController::OnSelectPrev);
}

void ABattlePlayerController::OnAttackPressed()
{
	bSelectingTarget = true;
	SelectedIndex = 0;

	EnemyUnits[SelectedIndex]->SetSelected(true);

	UE_LOG(LogTemp, Warning,
		TEXT("[PC] Attack Select Mode ON"));
	
}

void ABattlePlayerController::OnSelectNext()
{
	if (!bSelectingTarget) return;

	EnemyUnits[SelectedIndex]->SetSelected(false);

	SelectedIndex = (SelectedIndex + 1) % EnemyUnits.Num();

	// 새 선택 적용
	EnemyUnits[SelectedIndex]->SetSelected(true);

	UE_LOG(LogTemp, Warning,
		TEXT("[PC] Target -> %s"),
		*GetNameSafe(EnemyUnits[SelectedIndex]));
}

void ABattlePlayerController::OnSelectPrev()
{
	if (!bSelectingTarget) return;

	SelectedIndex =
		(SelectedIndex - 1 + EnemyUnits.Num()) % EnemyUnits.Num();

	UE_LOG(LogTemp, Warning,
		TEXT("[PC] Target -> %s"),
		*GetNameSafe(EnemyUnits[SelectedIndex]));
}


void ABattlePlayerController::OnConfirm()
{
	if (!bSelectingTarget || !ControlledUnit)
		return;

	bSelectingTarget = false;

	EnemyUnits[SelectedIndex]->SetSelected(false);

	ControlledUnit->RequestAttack();
}

void ABattlePlayerController::SetControlledUnit(ABattleUnitActor* Unit)
{
	
	ControlledUnit = Unit;
}

void ABattlePlayerController::SetEnemyUnits(const TArray<ABattleUnitActor*>& Enemies)
{
	EnemyUnits = Enemies;
}

void ABattlePlayerController::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Error, TEXT("### PC BeginPlay ###"));
	UE_LOG(LogTemp, Error,
		TEXT("### PC CLASS = %s ###"),
		*GetClass()->GetName());

	

	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		UE_LOG(LogTemp, Error, TEXT("### LocalPlayer OK ###"));

		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			UE_LOG(LogTemp, Error, TEXT("### Subsystem OK ###"));
			Subsystem->AddMappingContext(BattleIMC, 0);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("### Subsystem NULL ###"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("### LocalPlayer NULL ###"));
	}
}
