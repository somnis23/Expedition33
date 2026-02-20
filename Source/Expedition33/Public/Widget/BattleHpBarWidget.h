// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "BattleHpBarWidget.generated.h"

/**
 * 
 */
class UProgressBar;
class UTextBlock;
class ABattleUnitActor;
UCLASS()
class EXPEDITION33_API UBattleHpBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void BindToUnit(ABattleUnitActor* InUnit);
	
	UFUNCTION(BlueprintCallable)
	ABattleUnitActor* GetBoundUnit() const{return BoundUnit ;} 
	
protected:
	UPROPERTY(meta =(BindWidget))
	UProgressBar* PB_HP;
	UPROPERTY(meta =(BindWidget))
	UTextBlock* TXT_HP;
	UPROPERTY(meta =(BindWidget))
	UTextBlock* TXT_Name;
	
	UPROPERTY()
	ABattleUnitActor* BoundUnit = nullptr;
	
	UFUNCTION()
	void HandleHPChanged(int32 NewHP , int32 MaxHP);
	
};
