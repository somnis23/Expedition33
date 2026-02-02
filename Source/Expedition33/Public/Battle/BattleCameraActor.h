#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleCameraActor.generated.h"

class ABattleUnitActor;
class USpringArmComponent;
class UCameraComponent;

UCLASS()
class EXPEDITION33_API ABattleCameraActor : public AActor
{
	GENERATED_BODY()

public:
	ABattleCameraActor();
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

public:
	// === 초기화 ===
	void Init(ABattleUnitActor* InPlayer, ABattleUnitActor* InEnemy);
	
	UCameraComponent* GetCameraComponent() const {return Camera;}
protected:
	// === Components ===
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* Camera;

	// === Targets ===
	UPROPERTY()
	ABattleUnitActor* Player;

	UPROPERTY()
	ABattleUnitActor* Enemy;

	// === Camera Params ===
	UPROPERTY(EditAnywhere, Category="BattleCamera")
	float Height = 120.f;

	UPROPERTY(EditAnywhere, Category="BattleCamera")
	float DistanceMultiplier = 1.2f;

	UPROPERTY(EditAnywhere, Category="BattleCamera")
	float MinDistance = 300.f;

	UPROPERTY(EditAnywhere, Category="BattleCamera")
	float MaxDistance = 900.f;

	
public:
	
	
	
	void SetAnchor(AActor* InAnchor){Anchor = InAnchor;}
	
	UFUNCTION()
	void SetFreeAim(bool bEnable);
	
	void AddAimInput(float YawDelta , float PitchDelta);
	
	void UpdateBattleCam(float DT);
	void UpdateFreeAimCam(float DT);
	
	
private:
	void UpdateCameraTransform();
	
	TWeakObjectPtr<AActor> Anchor;

	bool bFreeAim = false;

	// 현재/목표 값(보간용)
	float TargetFOV;
	float TargetArmLength;
	FVector TargetOffset;

	float CurrentYaw = 0.f;
	float CurrentPitch = 0.f;

	// 기본/조준 파라미터
	float DefaultFOV = 90.f;
	float AimFOV = 60.f;

	float DefaultArm = 380.f;
	float AimArm = 260.f;

	FVector DefaultOffset = FVector::ZeroVector;
	FVector AimOffset = FVector(0.f, 35.f, 10.f);

	float BlendSpeed = 12.f;

	float PitchMin = -35.f;
	float PitchMax = 20.f;
};