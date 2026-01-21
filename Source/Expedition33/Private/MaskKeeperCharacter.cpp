// Fill out your copyright notice in the Description page of Project Settings.


#include "MaskKeeperCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AMaskKeeperCharacter::AMaskKeeperCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	GetCharacterMovement()->DisableMovement();
}

// Called when the game starts or when spawned
void AMaskKeeperCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame


// Called to bind functionality to input
void AMaskKeeperCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

