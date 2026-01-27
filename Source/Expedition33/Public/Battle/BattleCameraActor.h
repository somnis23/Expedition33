// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleCameraActor.generated.h"

UCLASS()
class EXPEDITION33_API ABattleCameraActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABattleCameraActor();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USpringArmComponent* SpringArm;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UCameraComponent* Camera;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite , Category="BattleCamera")
	FVector CameraOffset = FVector(-450.f , -250.f , 280.f);
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite , Category="BattleCamera")
	float BlendTime = 0.7f;
	
	void SetupForCenter(const FVector& Center);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
