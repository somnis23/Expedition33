// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/BattleHpBarWidget.h"

#include "BattleUnitActor.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UBattleHpBarWidget::BindToUnit(ABattleUnitActor* InUnit)
{
	BoundUnit = InUnit;
	
	if (!BoundUnit)
	{
		SetVisibility(ESlateVisibility::Hidden);
		return;
	}
	SetVisibility(ESlateVisibility::Visible);
	
	BoundUnit->OnHPChanged.AddDynamic(this , &UBattleHpBarWidget::HandleHPChanged);
	
	if (TXT_Name)
	{
		TXT_Name->SetText(BoundUnit->GetDisplayName());
	}
	HandleHPChanged(BoundUnit->GetHP() , BoundUnit->GetMaxHP());
	
}

void UBattleHpBarWidget::HandleHPChanged(int32 NewHP, int32 MaxHP)
{
	
	const float Ratio = (MaxHP > 0) ? (float)NewHP / (float)MaxHP : 0.f;
	if (PB_HP) PB_HP->SetPercent(Ratio);
	if (TXT_HP) TXT_HP->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), NewHP, MaxHP)));

}
