#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleUnitActor.generated.h"

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

	UPROPERTY(BlueprintReadOnly, Category="Battle")
	EBattleUnitState CurrentState = EBattleUnitState::Idle;

	// BP의 SkeletalMeshComponent를 런타임에 참조
	UPROPERTY(BlueprintReadOnly, Category="Battle")
	USkeletalMeshComponent* CharacterMeshComp = nullptr;

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
	
	void ApplyTurnAnim();
};