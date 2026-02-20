#include "BattleGameMode.h"
#include "BattleUnitActor.h"
#include "ExpeditionGameInstance.h"
#include "Battle/BattleCameraActor.h"
#include "Battle/BattlePlayerController.h"
#include "Kismet/GameplayStatics.h"

ABattleGameMode::ABattleGameMode()
{
    PrimaryActorTick.bCanEverTick = false;
   PlayerControllerClass = ABattlePlayerController::StaticClass();
    
    DefaultPawnClass = nullptr;
    
}

void ABattleGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Error,
    TEXT("### GAMEMODE CLASS = %s ###"),
    *GetClass()->GetName());
    
    if (UExpeditionGameInstance* GI = GetGameInstance<UExpeditionGameInstance>())
    {
        GI->PlayBGM(GI->BattleBGM , 0.5f);
        
    }
    
    SpawnPlayer();
    SpawnEnemy();
    AdjustBattleFormation(DesiredBattleDistance);
    GetWorldTimerManager().SetTimerForNextTick(this, &ABattleGameMode::SpawnBattleCamera);
    if (!BattlePlayer || !BattleEnemy)
    {
        UE_LOG(LogTemp, Error, TEXT("BattleGameMode: Failed to spawn battle units"));
        return;
    }

    // === Turn Manager ===
    TurnManager = GetWorld()->SpawnActor<ABattleTurnManager>();
    if (!TurnManager)
    {
        UE_LOG(LogTemp, Error, TEXT("BattleGameMode: Failed to spawn TurnManager"));
        return;
    }

    // === Register Units ===
    TArray<ABattleUnitActor*> Units;
    Units.Add(BattlePlayer);
    Units.Add(BattleEnemy);

    TurnManager->Initialize(Units);

    UE_LOG(LogTemp, Warning, TEXT("BattleGameMode: Battle Start"));
    TurnManager->StartBattle();
    
    ABattlePlayerController* PC = Cast<ABattlePlayerController>
    (UGameplayStatics::GetPlayerController(this,0));
    
}

void ABattleGameMode::RequestBattleEnd(bool bPlayerWin)
{
    if (bBattleEnding) return;
    bBattleEnding = true;
    
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.08f);
    
    FTimerDelegate Del;
    Del.BindUFunction(this, FName("FinishBattleEnd"));
    
    GetWorld()->GetTimerManager().SetTimerForNextTick([this, Del]()
    {
        GetWorld()->GetTimerManager().SetTimer(BattleEndTimerHandle, Del, 0.7f, false);
    });
}

void ABattleGameMode::FinishBattleEnd()
{
    
    // 1) 타임 복구
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);

    // 2) GameInstance에서 돌아갈 레벨 얻기
    if (UExpeditionGameInstance* GI = Cast<UExpeditionGameInstance>(GetGameInstance()))
    {
        const FName ReturnLevel = GI->ReturnLevelName;  
        if (!ReturnLevel.IsNone())
        {
            GI->StopBGM(2.0f);
            UGameplayStatics::OpenLevel(this, ReturnLevel);
            return;
        }
    }

    // fallback
    UGameplayStatics::OpenLevel(this, FName("Level_01_terrain"));
    
}

void ABattleGameMode::SpawnPlayer()
{
    UWorld* World = GetWorld();
    if (!World || !PlayerBattleClass) return;

    PlayerSpawnLocation = FVector(0.f, -200.f, 0.f);
    PlayerSpawnRotation = FRotator::ZeroRotator;

    BattlePlayer = World->SpawnActor<AMaelleBattleActor>(
        PlayerBattleClass,
        PlayerSpawnLocation,
        PlayerSpawnRotation
    );

    if (BattlePlayer)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
        if (PC)
        {
            PC->SetViewTarget(BattlePlayer);
        }

        UE_LOG(LogTemp, Warning, TEXT("BattleGameMode: Player spawned"));
    }
}

void ABattleGameMode::SpawnEnemy()
{
    UWorld* World = GetWorld();
    if (!World || !EnemyBattleClass) return;

    EnemySpawnLocation = FVector(400.f, 0.f, 0.f);
    EnemySpawnRotation = FRotator(0.f, 180.f, 0.f);

    BattleEnemy = World->SpawnActor<ABattleEnemyActor>(
        EnemyBattleClass,
        EnemySpawnLocation,
        EnemySpawnRotation
    );

    if (BattleEnemy)
    {
        UE_LOG(LogTemp, Warning, TEXT("BattleGameMode: Enemy spawned"));
    }
}

void ABattleGameMode::SpawnBattleCamera()
{
    UE_LOG(LogTemp, Warning, TEXT("[GM] SpawnBattleCamera called"));
    if (!BattleCameraClass || !BattlePlayer || !BattleEnemy)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnBattleCamera failed: invalid refs"));
        return;
    }
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    ABattleCameraActor* Cam =
        GetWorld()->SpawnActor<ABattleCameraActor>(
            BattleCameraClass,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            Params
        );

    Cam->Init(BattlePlayer, BattleEnemy);

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC)
    {
        PC->SetViewTargetWithBlend(Cam, 0.5f);
    }
    if (ABattlePlayerController* BPC = Cast<ABattlePlayerController>(PC))
    {
       BPC->SetBattleCam(Cam);
        UE_LOG(LogTemp, Warning, TEXT("[GM] BattleCam injected to PC: %s"), *GetNameSafe(Cam));
    }
    
}

void ABattleGameMode::AdjustBattleFormation(float DesiredDist)
{
    if (!BattlePlayer || !BattleEnemy) return;

    const FVector P = BattlePlayer->GetActorLocation();
    const FVector E = BattleEnemy->GetActorLocation();

    // 현재 중앙 고정 (카메라 Center 유지 포인트)
    const FVector Center = (P + E) * 0.5f;

    // 두 유닛을 잇는 방향(플레이어 -> 적)
    FVector Dir = (E - P);
    Dir.Z = 0.f;
    Dir = Dir.GetSafeNormal();
    if (Dir.IsNearlyZero()) Dir = FVector::ForwardVector;

    // 원하는 거리의 절반만큼 양쪽으로 벌림
    const float Half = DesiredDist * 0.5f;

    const FVector NewPlayerLoc = Center - Dir * Half;
    const FVector NewEnemyLoc  = Center + Dir * Half;

    BattlePlayer->SetActorLocation(NewPlayerLoc);
    BattleEnemy->SetActorLocation(NewEnemyLoc);

    /* 서로 바라보게
    BattlePlayer->SetActorRotation((NewEnemyLoc - NewPlayerLoc).Rotation());
    BattleEnemy->SetActorRotation((NewPlayerLoc - NewEnemyLoc).Rotation());*/

    UE_LOG(LogTemp, Warning, TEXT("[GM] AdjustBattleFormation Dist=%.1f Center=(%.1f,%.1f)"),
        DesiredDist, Center.X, Center.Y);
    
}
