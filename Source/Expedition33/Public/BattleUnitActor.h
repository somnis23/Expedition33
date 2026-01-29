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
	
	void ConfirmnAttack();
	
	void ApplyTurnAnim();
	
	UFUNCTION(BlueprintCallable)
	void RequestAttack();
	
	UFUNCTION(BlueprintCallable)
	virtual void SetSelected(bool bSelected);
	
};