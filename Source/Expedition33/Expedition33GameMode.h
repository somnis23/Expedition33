// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Expedition33GameMode.generated.h"

UCLASS(minimalapi)
class AExpedition33GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AExpedition33GameMode();
	
protected:
	virtual void BeginPlay();
};



