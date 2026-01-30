// Fill out your copyright notice in the Description page of Project Settings.


#include "Battle/BattlePlayerController.h"
#include "BattleGameMode.h"
#include "Battle/BattleTurnManager.h"
#include "BattleUnitActor.h"
#include "Kismet/GameplayStatics.h"

void ABattlePlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    check(InputComponent);

    // 키 바인딩 (Legacy Input 기준)
    InputComponent->BindKey(EKeys::S, IE_Pressed, this, &ABattlePlayerController::OnPressS_AttackMode);
    InputComponent->BindKey(EKeys::A, IE_Pressed, this, &ABattlePlayerController::OnPressA_PrevTarget);
    InputComponent->BindKey(EKeys::D, IE_Pressed, this, &ABattlePlayerController::OnPressD_NextTarget);
    InputComponent->BindKey(EKeys::F, IE_Pressed, this, &ABattlePlayerController::OnPressF_ConfirmTarget);
}

bool ABattlePlayerController::IsPlayerTurn() const
{
    ABattleUnitActor* Unit = GetCurrentUnit();
    return Unit && Unit->IsMyTurn() && Unit->IsPlayerUnit(); 
}

ABattleUnitActor* ABattlePlayerController::GetCurrentUnit() const
{
    if (UWorld* World = GetWorld())
    {
        if (ABattleGameMode* GM = World->GetAuthGameMode<ABattleGameMode>())
        {
            if (ABattleTurnManager* TM = GM->GetTurnManager())
            {
                return TM->GetCurrentUnit(); // 없으면 만들어
            }
        }
    }
    return nullptr;
}

void ABattlePlayerController::CacheEnemies()
{
    CachedEnemies.Reset();

    // TurnManager에서 적 리스트를 주는 게 베스트(없으면 GetAllActorsOfClass로 임시)
    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABattleUnitActor::StaticClass(), Found);

    for (AActor* A : Found)
    {
        ABattleUnitActor* Unit = Cast<ABattleUnitActor>(A);
        if (!Unit) continue;
        if (Unit->IsEnemyUnit() && Unit->IsAlive()) // 너 프로젝트 기준으로 수정
        {
            CachedEnemies.Add(Unit);
        }
    }
}

void ABattlePlayerController::ClearEnemySelection()
{
    for (ABattleUnitActor* E : CachedEnemies)
    {
        if (E) E->SetSelected(false); // 너가 이미 쓰는 Selected 표시 함수/변수로 연결
    }
}

void ABattlePlayerController::ApplyEnemySelection()
{
    if (CachedEnemies.Num() == 0) return;
    SelectedEnemyIndex = (SelectedEnemyIndex + CachedEnemies.Num()) % CachedEnemies.Num();

    ClearEnemySelection();
    if (CachedEnemies[SelectedEnemyIndex])
    {
        CachedEnemies[SelectedEnemyIndex]->SetSelected(true);
        UE_LOG(LogTemp, Warning, TEXT("[PC] Target Selected = %s"), *CachedEnemies[SelectedEnemyIndex]->GetName());
    }
}

void ABattlePlayerController::OnPressS_AttackMode()
{
    if (!IsPlayerTurn()) return;

    if (InputMode == EBattleInputMode::TargetSelect)
    {
        InputMode = EBattleInputMode::None;
        ClearEnemySelection();
        UE_LOG(LogTemp, Warning, TEXT("[PC] Attack Select Mode OFF"));
        return;
    }

    CacheEnemies();
    if (CachedEnemies.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PC] No Enemies to Select"));
        return;
    }

    InputMode = EBattleInputMode::TargetSelect;
    SelectedEnemyIndex = 0;
    ApplyEnemySelection();

    UE_LOG(LogTemp, Warning, TEXT("[PC] Attack Select Mode ON"));
}

void ABattlePlayerController::OnPressA_PrevTarget()
{
    if (InputMode != EBattleInputMode::TargetSelect) return;
    SelectedEnemyIndex--;
    ApplyEnemySelection();
}

void ABattlePlayerController::OnPressD_NextTarget()
{
    if (InputMode != EBattleInputMode::TargetSelect) return;
    SelectedEnemyIndex++;
    ApplyEnemySelection();
}

void ABattlePlayerController::OnPressF_ConfirmTarget()
{
    if (InputMode != EBattleInputMode::TargetSelect) return;
    if (!IsPlayerTurn()) return;
    if (CachedEnemies.Num() == 0) return;

    ABattleUnitActor* Attacker = GetCurrentUnit();
    ABattleUnitActor* Target = CachedEnemies[SelectedEnemyIndex];

    if (!Attacker || !Target) return;

    // 확정: 선택 해제 + 모드 종료
    ClearEnemySelection();
    InputMode = EBattleInputMode::None;

    UE_LOG(LogTemp, Warning, TEXT("[PC] Confirm Target=%s -> RequestAttack"), *Target->GetName());

    Attacker->SetCurrentTarget(Target); // 없으면 추가
    Attacker->RequestAttack();          // 여기서 AnimBP에 bAttackRequest 쏴야 함
}
