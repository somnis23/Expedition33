#include "BattleGameMode.h"
#include "BattleUnitActor.h"
#include "Battle/BattleCameraActor.h"
#include "Kismet/GameplayStatics.h"

ABattleGameMode::ABattleGameMode()
{
    PrimaryActorTick.bCanEverTick = false;
    DefaultPawnClass = nullptr;
}

void ABattleGameMode::BeginPlay()
{
    Super::BeginPlay();

    SpawnPlayer();
    SpawnEnemy();
    
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
    
}
