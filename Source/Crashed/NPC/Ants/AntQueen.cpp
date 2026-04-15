#include "AntQueen.h"
#include "ForestAnt.h"
#include "AntHive.h"
#include "Crashed/HealthComponent.h"

#include "Engine/World.h"

AAntQueen::AAntQueen()
{
    DetectionRadius = 0.f;
}

void AAntQueen::ReceiveFood(float Amount)
{
    FoodSupply += Amount;
}


void AAntQueen::BeginPlay()
{
    Super::BeginPlay();

    if (HealthComponent)
    {
        HealthComponent->OnHealthChanged.AddDynamic(this, &AAntQueen::OnQueenDamaged);
    }

    GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AAntQueen::CheckAndSpawn, SpawnInterval, true, SpawnInterval);

    SpawnInitialHealers();
}


// Routine spawning — maintains worker/soldier ratio
void AAntQueen::CheckAndSpawn()
{
    PurgeDeadEntries();

    const int32 NumWorkers  = AliveWorkers.Num();
    const int32 NumSoldiers = AliveSoldiers.Num();

      
    if (NumSoldiers == 0)
    {
        SpawnAnt(SoldierAntClass, AliveSoldiers);
    }
    
    const float CurrentRatio =(float)NumWorkers / (float)NumSoldiers;

    if (NumSoldiers < MaxSoldiers && CurrentRatio >= WorkerToSoldierRatio)
    {
        SpawnAnt(SoldierAntClass, AliveSoldiers);
    }
    else if (NumWorkers < MaxWorkers)
    {
        if (AliveWorkers.Num() < MinWorkersBeforeFoodCost)
        {
            SpawnAnt(WorkerAntClass, AliveWorkers);
        }
        else if (FoodSupply >= FoodCostPerSpawn)
        {
            FoodSupply -= FoodCostPerSpawn;
            SpawnAnt(WorkerAntClass, AliveWorkers);
        }
        // else: not enough food, skip worker spawn this tick
    }

    SpawnInterval = FMath::FRandRange(3.f, 8.f);
}

void AAntQueen::SpawnAnt(TSubclassOf<AForestAnt> AntClass, TArray<TWeakObjectPtr<AForestAnt>>& TrackingArray)
{
    if (!AntClass) return;

    const float Angle  = FMath::FRandRange(0.f, 2.f * PI);
    const float Radius = FMath::FRandRange(100.f, SpawnRadius);
    const FVector SpawnLocation = GetActorLocation() + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.f);

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AForestAnt* NewAnt = GetWorld()->SpawnActor<AForestAnt>(AntClass, SpawnLocation, FRotator::ZeroRotator, Params);
    if (NewAnt)
    {
        NewAnt->RegisterWithQueen(this);
        NewAnt->SetHomeHive(HomeHive);  // pass hive reference so ant can return home
        NewAnt->PatrolPath = AntPatrolPath; 
        
        TrackingArray.Add(NewAnt);
    }
}

void AAntQueen::PurgeDeadEntries()
{
    AliveWorkers.RemoveAll( [](const TWeakObjectPtr<AForestAnt>& P){ return !P.IsValid(); });
    AliveSoldiers.RemoveAll([](const TWeakObjectPtr<AForestAnt>& P){ return !P.IsValid(); });
    AliveHealers.RemoveAll( [](const TWeakObjectPtr<AForestAnt>& P){ return !P.IsValid(); });
}

// -----------------------------------------------------------------------
// Death callback — removes ant from whichever array it belongs to
// -----------------------------------------------------------------------

void AAntQueen::OnAntDied(ABaseEnemy* DeadAnt)
{
    AForestAnt* Dead = Cast<AForestAnt>(DeadAnt);
    if (!Dead) return;

    AliveWorkers.RemoveAll( [Dead](const TWeakObjectPtr<AForestAnt>& P){ return P.Get() == Dead; });
    AliveSoldiers.RemoveAll([Dead](const TWeakObjectPtr<AForestAnt>& P){ return P.Get() == Dead; });

    const bool bWasHealer = AliveHealers.ContainsByPredicate(
        [Dead](const TWeakObjectPtr<AForestAnt>& P){ return P.Get() == Dead; });

    if (bWasHealer)
    {
        AliveHealers.RemoveAll([Dead](const TWeakObjectPtr<AForestAnt>& P){ return P.Get() == Dead; });
        ScheduleHealerReplacement();
    }
}

// -----------------------------------------------------------------------
// Healer management
// -----------------------------------------------------------------------

void AAntQueen::SpawnInitialHealers()
{
    for (int32 i = 0; i < InitialHealerCount; ++i)
    {
        SpawnAnt(HealerAntClass, AliveHealers);
    }
}

void AAntQueen::ScheduleHealerReplacement()
{
    // Each replacement gets its own one-shot timer
    FTimerHandle TempHandle;
    GetWorldTimerManager().SetTimer(TempHandle, this, &AAntQueen::SpawnReplacementHealer, HealerReplaceDelay, false);
}

void AAntQueen::SpawnReplacementHealer()
{
    SpawnAnt(HealerAntClass, AliveHealers);
}


// Health-phase triggers on % thresholds
void AAntQueen::OnQueenDamaged(float NewHealth, float MaxHealth)
{
    if (MaxHealth <= 0.f) return;

    const float Pct = NewHealth / MaxHealth;

    if (!bThreshold66Triggered && Pct <= 0.66f)
    {
        bThreshold66Triggered = true;

        for (int32 i = 0; i < SoldiersSpawnedAt66; ++i)
            SpawnAnt(SoldierAntClass, AliveSoldiers);

        // Extra set of 3 healers
        for (int32 i = 0; i < 3; ++i)
            SpawnAnt(HealerAntClass, AliveHealers);
    }

    if (!bThreshold33Triggered && Pct <= 0.33f)
    {
        bThreshold33Triggered = true;

        for (int32 i = 0; i < SoldiersSpawnedAt33; ++i)
            SpawnAnt(SoldierAntClass, AliveSoldiers);

        // Extra set of 3 healers
        for (int32 i = 0; i < 3; ++i)
            SpawnAnt(HealerAntClass, AliveHealers);
    }
}