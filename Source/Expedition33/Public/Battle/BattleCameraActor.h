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

protected:
	virtual void BeginPlay() override;

public:
	// === 초기화 ===
	void Init(ABattleUnitActor* InPlayer, ABattleUnitActor* InEnemy);

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

private:
	void UpdateCameraTransform();
};