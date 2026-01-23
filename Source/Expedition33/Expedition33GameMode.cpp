// Copyright Epic Games, Inc. All Rights Reserved.

#include "Expedition33GameMode.h"
#include "Expedition33Character.h"
#include "ExpeditionGameInstance.h"
#include "UObject/ConstructorHelpers.h"

AExpedition33GameMode::AExpedition33GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void AExpedition33GameMode::BeginPlay()
{
	Super::BeginPlay();
	
	if (UExpeditionGameInstance* GI = GetGameInstance<UExpeditionGameInstance>())
	{
		GI->PlayBGM(GI->WorldBGM,1.0f);
	}
	
}
