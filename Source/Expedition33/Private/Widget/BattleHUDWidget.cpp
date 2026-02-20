// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/BattleHUDWidget.h"

void UBattleHUDWidget::SetHintState(EBattleHUDHintState State)
{
	if (Panel_Default)      Panel_Default->SetVisibility(ESlateVisibility::Collapsed);
	if (Panel_AttackSelect) Panel_AttackSelect->SetVisibility(ESlateVisibility::Collapsed);
	if (Panel_SkillSelect)  Panel_SkillSelect->SetVisibility(ESlateVisibility::Collapsed);

	switch (State)
	{
	case EBattleHUDHintState::Default:
		if (Panel_Default) Panel_Default->SetVisibility(ESlateVisibility::Visible);
		break;
	case EBattleHUDHintState::AttackSelect:
		if (Panel_AttackSelect) Panel_AttackSelect->SetVisibility(ESlateVisibility::Visible);
		break;
	case EBattleHUDHintState::SkillSelect:
		if (Panel_SkillSelect) Panel_SkillSelect->SetVisibility(ESlateVisibility::Visible);
		break;
	}
	
}
