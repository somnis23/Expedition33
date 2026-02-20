// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "BattleHUDWidget.generated.h"

/**
 * 
 */
UENUM(Blueprintable)
enum class EBattleHUDHintState  : uint8
{
	Default ,
	AttackSelect , 
	SkillSelect 
};
UCLASS()
class EXPEDITION33_API UBattleHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SetHintState(EBattleHUDHintState State);
	
	protected:
	UPROPERTY(meta=(BindWidget))
	class UWidget* Panel_Default;
	
	UPROPERTY(meta=(BindWidget))
	class UWidget* Panel_AttackSelect;
	
	UPROPERTY(meta=(BindWidget))
	class UWidget* Panel_SkillSelect;
	
};
