#pragma once

#include "CoreMinimal.h"
#include "Battle/BattleTypes.h"
#include "GameFramework/Actor.h"
#include "BattleUnitActor.generated.h"

UENUM(BlueprintType)
enum class EBattleUnitType : uint8
{
	Player,
	Enemy
};


UENUM(BlueprintType)
enum class EBattleUnitState : uint8
{
	Idle,
	Attack,
	Hit,
	Dead
};

class USkeletalMeshComponent;
class UBattleAnimInstance;

UCLASS()
class EXPEDITION33_API ABattleUnitActor : public AActor
{
	GENERATED_BODY()

public:
	ABattleUnitActor();
public:
	// PC가 "내 턴인지" 확인할 수 있게
	FORCEINLINE bool IsMyTurn() const { return bIsMyTurn; }

	// 유닛 타입 판정
	FORCEINLINE bool IsPlayerUnit() const { return UnitType == EBattleUnitType::Player; }
	FORCEINLINE bool IsEnemyUnit()  const { return UnitType == EBattleUnitType::Enemy; }

	// 생존 판정(프로젝트 규칙에 맞게 수정 가능)
	FORCEINLINE bool IsAlive() const { return CurrentState != EBattleUnitState::Dead; }

	// 타겟 지정/조회
	FORCEINLINE void SetCurrentTarget(ABattleUnitActor* InTarget) { SelectedTarget = InTarget; }
	FORCEINLINE ABattleUnitActor* GetCurrentTarget() const { return SelectedTarget; }

	// 커맨드 상태 지정/조회(선택모드 유지에 필요)
	FORCEINLINE void SetCommandState(EBattleCommandState NewState) { CommandState = NewState; }
	FORCEINLINE EBattleCommandState GetCommandState() const { return CommandState; }
	
	
protected:
	virtual void BeginPlay() override;

	bool bIsMyTurn = false;
	
	UPROPERTY(EditDefaultsOnly , BlueprintReadOnly , Category="Battle")
	EBattleUnitType UnitType = EBattleUnitType::Enemy;
	

	UPROPERTY(BlueprintReadOnly, Category="Battle")
	EBattleUnitState CurrentState = EBattleUnitState::Idle;

	// BP의 SkeletalMeshComponent를 런타임에 참조
	UPROPERTY(BlueprintReadOnly, Category="Battle")
	USkeletalMeshComponent* CharacterMeshComp = nullptr;
	
	// 전투 입력 상태 
	UPROPERTY(BlueprintReadOnly, Category="Battle")
	EBattleCommandState CommandState = EBattleCommandState::None;
	
	UPROPERTY(BlueprintReadOnly, Category="Battle")
	ABattleUnitActor* SelectedTarget = nullptr;

public:
	UFUNCTION(BlueprintCallable)
	void SetBattleState(EBattleUnitState NewState);

	UFUNCTION(BlueprintCallable)
	virtual void OnTurnStart();

	UFUNCTION(BlueprintCallable)
	virtual void OnTurnEnd();

	UFUNCTION(BlueprintCallable)
	void OnTurnStateChanged(
		EBattleTurnState TurnState,
		bool bMyTurn,
		bool bTurnStart
	);
	
	UFUNCTION(BlueprintCallable)
	void EnterAttackMode();
	
	void ConfirmAttack();
	
	void ApplyTurnAnim();
	
	UFUNCTION(BlueprintCallable)
	void RequestAttack();
	
	void OnAttackFinished();
	
	UFUNCTION(BlueprintCallable)
	virtual void SetSelected(bool bSelected);
	
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//=========================
	// Enemy battle 연출용
	//=========================
	UPROPERTY(EditDefaultsOnly , Category="EnemyTurn")
	float EnemyWalkDistance = 120.f;
	
	UPROPERTY(EditDefaultsOnly , Category="EnemyTurn")
	float EnemyWalkDuration = 0.4f;
	
	UPROPERTY(EditDefaultsOnly , Category="EnemyTurn")
	float EnemyChargeDuration = 0.6f;
	
	UPROPERTY(EditDefaultsOnly, Category="EnemyTurn")
	float EnemyRetreatDuration = 0.45f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="EnemyTurn")
	EEnemyTurnPhase EnemyPhaseRuntime = EEnemyTurnPhase::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="EnemyTurn")
	EEnemyAttackType PlannedAttack = EEnemyAttackType::Fire;
	
	
	// 원위치 복귀용 위치
	FVector EnemyHomeLocation = FVector::ZeroVector;
	
	bool bWalking = false;
	FVector WalkStartLoc = FVector::ZeroVector;
	FVector WalkTargetLoc = FVector::ZeroVector;
	float WalkElapsed =0.f;
	
	bool bRetreating = false;
	float RetreatElapsed = 0.f;
	FVector RetreatStartLoc = FVector::ZeroVector;
	
	USkeletalMeshComponent* GetCharacterMesh() const { return CharacterMeshComp; }
	
	FTimerHandle EnemyPhaseTimer;
	
	//ap > 코스트 시스템
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Battle|Cost")
	int32 MaxCost = 9;

	UPROPERTY(BlueprintReadOnly, Category="Battle|Cost")
	int32 Cost = 0;

	UFUNCTION(BlueprintCallable, Category="Battle|Cost")
	void RefillCostForTurn() { Cost = MaxCost; }

	UFUNCTION(BlueprintCallable, Category="Battle|Cost")
	bool CanSpendCost(int32 Amount) const { return bIsMyTurn && Cost >= Amount; }

	UFUNCTION(BlueprintCallable, Category="Battle|Cost")
	bool SpendCost(int32 Amount);
	
	UFUNCTION(BlueprintCallable)
	void OnFreeAimHit(const FName HitBone);
	
	//void OnFreeAimHit(FName HitBone);
	
	void StartEnemyWalk();
	void StartEnemyCharge();
	void StartEnemyAttack();
	void StartEnemyRetreat();
	void SetEnemyPhaseOnAnim(EEnemyTurnPhase Phase);
	void SetPlannedAttackOnAnim(EEnemyAttackType Type);
	
	
};