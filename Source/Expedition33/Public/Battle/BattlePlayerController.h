// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BattleCameraActor.h"
#include "BattleUnitActor.h"
#include "Expedition33/Expedition33Character.h"
#include "CineCameraComponent.h"
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
class ACineCameraActor;
class UCameraShakeBase;
class UCineCameraComponent;

UCLASS()
class EXPEDITION33_API ABattlePlayerController : public APlayerController
{
	GENERATED_BODY()


public:
	virtual void SetupInputComponent() override;
	
	
	UPROPERTY()
	ABattleCameraActor* BattleCameraActor = nullptr;
	
	UFUNCTION()
	void SetBattleCam(ABattleCameraActor* InCam) {BattleCam = InCam;}
	
	
	UCameraComponent* GetBattleCameraComponent() const
	{
		return BattleCameraActor ? BattleCameraActor->GetBattleCameraComponent() : nullptr;
	}
	
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
	
	/*
	UCineCameraComponent* GetCineCameraComponent() const { return Camera; }
	UCineCameraComponent* GetCameraComponent() const { return Camera; }*/
	
	ABattleCameraActor* GetBattleCam();
	
	TWeakObjectPtr<ABattleUnitActor> AimedUnit;
	FHitResult LastAimHit;
	
	float AimTraceDistance = 200000.f;
	
	UPROPERTY(EditAnywhere,Category="BattleCost")
	int32 FreeAimShotCost = 1;
	
	// 풀 패링 카운터
	UPROPERTY() 
	class ACineCameraActor* CounterCam = nullptr;
	/*UPROPERTY(VisibleAnywhere)
	UCineCameraComponent* Camera;*/
	
	/*UPROPERTY() 
	AActor* CounterPlayer = nullptr;
	UPROPERTY() 
	AActor* CounterEnemy  = nullptr;*/

	bool  bCounterCamActive = false;
	float CounterElapsed  = 0.f;

	float CounterDuration = 3.35f; 
	
	UPROPERTY()
	AActor* CounterReturnTarget = nullptr;

	// === Counter Cam Tunables (video-like) ===
	UPROPERTY(EditDefaultsOnly, Category="Camera|Counter")
	float BlendInTime = 0.12f;

	UPROPERTY(EditDefaultsOnly, Category="Camera|Counter")
	float BlendOutTime = 0.10f;

	UPROPERTY(EditDefaultsOnly, Category="Camera|Counter")
	float PivotHeight = 85.f;     // 중간 피벗 높이

	UPROPERTY(EditDefaultsOnly, Category="Camera|Counter")
	float HeightOffset = 40.f;    // 카메라 자체 높이 오프셋

	UPROPERTY(EditDefaultsOnly, Category="Camera|Counter")
	float StartDistance = 170.f;

	UPROPERTY(EditDefaultsOnly, Category="Camera|Counter")
	float WideDistance  = 330.f;

	UPROPERTY(EditDefaultsOnly, Category="Camera|Counter")
	float PunchDistance = 190.f;

	UPROPERTY(EditDefaultsOnly, Category="Camera|Counter")
	float StartSides = 140.f;

	UPROPERTY(EditDefaultsOnly, Category="Camera|Counter")
	float WideSides  = 210.f;

	UPROPERTY(EditDefaultsOnly, Category="Camera|Counter")
	float PunchSides = 160.f;

	UPROPERTY(EditDefaultsOnly, Category="Camera|Counter")
	float StartFOV = 52.f;

	UPROPERTY(EditDefaultsOnly, Category="Camera|Counter")
	float WideFOV  = 42.f;

	UPROPERTY(EditDefaultsOnly, Category="Camera|Counter")
	float PunchFOV = 40.f;

	UPROPERTY(EditDefaultsOnly, Category="Camera|Counter")
	float RecoverFOV = 48.f;

	UPROPERTY(EditDefaultsOnly, Category="Camera|Counter")
	float OrbitDegrees = 15.f;

	UPROPERTY(EditDefaultsOnly, Category="Camera|Counter")
	TSubclassOf<UCameraShakeBase> CounterImpactShake;

	//---------------------------------------------------
	UPROPERTY(EditDefaultsOnly, Category="CounterCam")
	float CounterBlendIn = 0.08f;

	UPROPERTY(EditDefaultsOnly, Category="CounterCam")
	float CounterBlendOut = 0.10f;

	UPROPERTY(EditDefaultsOnly, Category="CounterCam")
	float CounterCamDistance = 520.f;

	UPROPERTY(EditDefaultsOnly, Category="CounterCam")
	float CounterCamHeight = 110.f;

	UPROPERTY(EditDefaultsOnly, Category="CounterCam")
	float CounterCamSide = -210.f; // (-)면 Player 기준 왼쪽

	UPROPERTY(EditDefaultsOnly, Category="CounterCam")
	float CounterLookAtZ = 90.f;

	UPROPERTY(EditDefaultsOnly, Category="CounterCam")
	float CounterMoveDuration = 0.10f; // "자리잡기" 시간

	// Counter slow (impact)
	UPROPERTY(EditDefaultsOnly, Category="CounterCam")
	float CounterImpactDilation = 0.14f;

	UPROPERTY(EditDefaultsOnly, Category="CounterCam")
	float CounterImpactSlowReal = 0.16f;

	
	bool bCounterCamSettled = false;
	float CounterStartRealTime = 0.f;

	TWeakObjectPtr<class ABattleUnitActor> CounterPlayer;
	TWeakObjectPtr<class ABattleUnitActor> CounterEnemy;

	FVector CounterFromLoc;
	FRotator CounterFromRot;

	AActor* SavedViewTarget = nullptr;

	/*UPROPERTY(EditDefaultsOnly, Category="CounterCam")
	AActor* BattleCameraActor = nullptr; // BP_BattleCameraActor 잡아두기*/

	void StartCounterCamera(class ABattleUnitActor* PlayerUnit, class ABattleUnitActor* EnemyUnit);
	void TriggerCounterImpact();
	void EndCounterCamera(bool bBlendBack);

	void UpdateCounterCam(); // Tick에서
	
	
	
	
	//---------------------------------------------------
	bool bImpactTriggered = false;
	
	//UFUNCTION(BlueprintCallable, Category="Camera")
	//void StartCounterCamera(AActor* PlayerActor , AActor* EnemyActor);
	

	
	
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
	
	
	void UpdateCounterCam_Cinematic(float DeltaTime);
	
	
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
