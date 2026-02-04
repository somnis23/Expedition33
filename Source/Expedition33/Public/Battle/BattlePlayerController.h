// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BattleUnitActor.h"
#include "Expedition33/Expedition33Character.h"
#include "GameFramework/PlayerController.h"
#include "BattlePlayerController.generated.h"

/**
 * 
 */
UENUM()
enum class EBattleInputMode : uint8
{
	None,
	TargetSelect,
	FreeAim
};
class ABattleUnitActor;
class ABattleTurnManager;
class ABattleGameMode;


UCLASS()
class EXPEDITION33_API ABattlePlayerController : public APlayerController
{
	GENERATED_BODY()


public:
	virtual void SetupInputComponent() override;
	
	UFUNCTION()
	void SetBattleCam(ABattleCameraActor* InCam) {BattleCam = InCam;}
	
	
	UPROPERTY()
	class ABattleCameraActor* BattleCam = nullptr;
	UPROPERTY(EditAnywhere, Category="FreeAim|Input")
	float AimYawSensitivity = 0.8f;     // 

	UPROPERTY(EditAnywhere, Category="FreeAim|Input")
	float AimPitchSensitivity = 0.8f;   // 
	
	UPROPERTY(EditAnywhere, Category="FreeAim|Input")
	bool bInvertX = false;              // 좌/우 

	UPROPERTY(EditAnywhere, Category="FreeAim|Input")
	bool bInvertY = true;               // 상/하 

	UPROPERTY(EditAnywhere, Category="FreeAim|Input")
	float AimDeadZone = 0.1f;           // 미세 떨림 
	
	UPROPERTY(EditDefaultsOnly , Category="UI")
	TSubclassOf<UUserWidget> FreeAimCrosshairClass;
	
	UPROPERTY()
	UUserWidget* FreeAimCrosshairWidget = nullptr;
	
	ABattleCameraActor* GetBattleCam();
	
	TWeakObjectPtr<ABattleUnitActor> AimedUnit;
	FHitResult LastAimHit;
	
	float AimTraceDistance = 200000.f;
	
	UPROPERTY(EditAnywhere,Category="BattleCost")
	int32 FreeAimShotCost = 1;
	
	
	// 매 프레임 조준 업데이트
	virtual void PlayerTick(float DeltaTime) override;

	bool TraceFromCameraCenter(FHitResult& OutHit) const;
	void UpdateAimTarget();                 // 하이라이트 갱신
	void ClearAimHighlight();
	

private:
	EBattleInputMode InputMode = EBattleInputMode::None;

	int32 SelectedEnemyIndex = 0;

	UPROPERTY()
	TArray<TObjectPtr<ABattleUnitActor>> CachedEnemies;

	bool IsPlayerTurn() const;
	ABattleUnitActor* GetCurrentUnit() const;
	void CacheEnemies();
	
	//freeaim
	
	

	void ClearEnemySelection();
	void ApplyEnemySelection();

	void OnPressS_AttackMode();
	void OnPressA_PrevTarget();
	void OnPressD_NextTarget();
	void OnPressF_ConfirmTarget();
	
	void OnFreeAimStart();
	void OnFreeAimEnd();
	
	void OnFreeAimFire();
	
	void OnParry();
	void OnDodge();
	ABattleUnitActor* GetPlayerUnit()const;
	
	
	
};
