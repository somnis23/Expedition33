#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleCameraActor.generated.h"

class ABattleUnitActor;
class USpringArmComponent;
class UCameraComponent;

UENUM()
enum class EGommageBeat : uint8
{
	None,
	Start,
	Hold,
	Impact,
	End
};
UENUM()
enum class ECinematicType : uint8
{
	None,
	Phantom,
	Gommage
};
UENUM()
enum class EGommageCamBeat : uint8
{
	None,
	Start,
	Focus,
	Impact,
	End
};
UENUM()
enum class EPhantomCamBeat : uint8
{
	None,
	Start,
	Dash,
	Impact,
	End
};
UENUM(BlueprintType)
enum class EBattleCamMode : uint8
{
	Battle,        // 기본
	TargetSelect,  // 타겟 선택(선택 적에 포커스)
	FreeAim
};

UCLASS()
class EXPEDITION33_API ABattleCameraActor : public AActor
{
	GENERATED_BODY()

public:
	ABattleCameraActor();
	virtual void Tick(float DeltaSeconds) override;
	
	UPROPERTY() 
	ECinematicType CineType = ECinematicType::None;
protected:
	virtual void BeginPlay() override;


public:
	// === 초기화 ===
	void Init(ABattleUnitActor* InPlayer, ABattleUnitActor* InEnemy);
	
	UPROPERTY(BlueprintReadOnly , Category="Cam")
	EBattleCamMode CamMode;
	UPROPERTY()
	TWeakObjectPtr<ABattleUnitActor> FocusTarget;
	
	UCameraComponent* GetBattleCameraComponent() const { return Camera; } 
	
	UFUNCTION(BlueprintCallable)
	void SetTargetSelect(bool bEnable , ABattleUnitActor* InTarget);
	
	
	UCameraComponent* GetCameraComponent() const {return Camera;}
protected:
	// === Components ===
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* Camera;

	UPROPERTY(BlueprintReadOnly, Category="Camera")
	bool bCinematicOverride = false;


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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BattleFreeAim")
	float FreeAimPivotZ = 140.f;   // ← 여기 중요 (130~170 사이 추천)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BattleFreeAim")
	float BattlePivotZ = 120.f;
	
public:
	
	
	
	void SetAnchor(AActor* InAnchor){Anchor = InAnchor;}
	
	UFUNCTION()
	void SetFreeAim(bool bEnable);
	
	void AddAimInput(float YawDelta , float PitchDelta);
	
	void UpdateBattleCam(float DT);
	void UpdateFreeAimCam(float DT);
	void UpdateTargetSelectCam(float DT);
	
	UFUNCTION(BlueprintCallable, Category="Camera")
	void SetCinematicOverride(bool bEnable) { bCinematicOverride = bEnable; }
	UFUNCTION(BlueprintCallable, Category="Camera")
	bool IsCinematicOverride() const { return bCinematicOverride; }
	
private:
	void UpdateCameraTransform();
	
	TWeakObjectPtr<AActor> Anchor;

	bool bFreeAim = false;
	bool bFreeAimJustEntered = false;

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
	float AimArm = 320.f;

	FVector DefaultOffset = FVector::ZeroVector;
	FVector AimOffset = FVector(0.f, 80.f, 10.f);

	float BlendSpeed = 12.f;

	float PitchMin = -35.f;
	float PitchMax = 20.f;
	
	// 팬텀스트라이크
	void UpdatePhantomStrikeCam(float DT);
	void PhantomGotoBeat(EPhantomCamBeat NextBeat, float Duration);
public:
	
	// 팬텀스트라이크 
	UPROPERTY()
	EPhantomCamBeat PhantomBeat = EPhantomCamBeat::None;

	UPROPERTY()
	float PhantomBeatTime = 0.f;

	UPROPERTY()
	float PhantomBeatDuration = 0.2f;

	UPROPERTY()
	float PhantomStartYaw = 0.f;

	UPROPERTY()
	float PhantomStartPitch = -20.f;

	UPROPERTY()
	float PhantomStartFOV = 70.f;

	UPROPERTY()
	float PhantomStartArm = 600.f;

	UPROPERTY()
	FVector PhantomStartOffset = FVector::ZeroVector;

	UPROPERTY()
	TWeakObjectPtr<class ABattleUnitActor> PhantomAttacker;

	UPROPERTY()
	TWeakObjectPtr<class ABattleUnitActor> PhantomTarget;
	
	UFUNCTION(BlueprintCallable)
	void StartPhantomStrikeCam(ABattleUnitActor* Attacker, ABattleUnitActor* Target);

	UFUNCTION(BlueprintCallable)
	void PhantomStrikeImpactBeat(); // 임팩트 순간(슬로우/줌/셰이크)

	UFUNCTION(BlueprintCallable)
	void EndPhantomStrikeCam();
	
	
public:
	UPROPERTY() 
	EGommageCamBeat GommageBeats = EGommageCamBeat::None;
	
	UPROPERTY()
	EGommageBeat GommageBeat = EGommageBeat::None;
	UPROPERTY() 
	float GommageT  = 0.f;
	UPROPERTY() 
	float GommageDur  = 0.2f;
	UPROPERTY() 
	TWeakObjectPtr<ABattleUnitActor> GommageCaster;
	UPROPERTY() 
	TWeakObjectPtr<ABattleUnitActor> GommageTarget;
	
	void StartGommageCam(ABattleUnitActor* Caster, ABattleUnitActor* Target);
	void GommageFocusBeat();
	void GommageImpactBeat();
	void EndGommageCam();
	void UpdateGommageCam(float DT);
	
};