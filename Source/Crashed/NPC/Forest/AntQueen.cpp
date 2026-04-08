#include "AntQueen.h"
#include "ForestAnt.h"
#include "Crashed/HealthComponent.h"

#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

AAntQueen::AAntQueen()
{
    // Queen herself doesn't chase the player — she stays put and defends
    DetectionRadius = 0.f;
}

void AAntQueen::BeginPlay()
{
    Super::BeginPlay();

    if (HealthComponent)
    {
        HealthComponent->OnHealthChanged.AddDynamic(this, &AAntQueen::OnQueenDamaged);
    }

    GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AAntQueen::CheckAndSpawn, SpawnInterval, true, SpawnInterval);
}

void AAntQueen::CheckAndSpawn()
{
    PurgeDeadEntries();

    const int32 NumWorkers  = AliveWorkers.Num();
    const int32 NumSoldiers = AliveSoldiers.Num();

    // Decide what to spawn: maintain ratio, obey max caps
    // If soldiers are 0 we always need at least one to anchor the ratio check
    const float CurrentRatio = (NumSoldiers > 0) ? (float)NumWorkers / (float)NumSoldiers : 0.f;

    if (NumSoldiers < MaxSoldiers && CurrentRatio >= WorkerToSoldierRatio)
    {
        // Ratio is met or exceeded — spawn a soldier
        SpawnAnt(SoldierAntClass, AliveSoldiers);
    }
    else if (NumWorkers < MaxWorkers)
    {
        SpawnAnt(WorkerAntClass, AliveWorkers);
    }
}

void AAntQueen::SpawnAnt(TSubclassOf<AForestAnt> AntClass, TArray<TWeakObjectPtr<AForestAnt>>& TrackingArray)
{
    if (!AntClass) return;

    // Random point on a circle around the queen
    const float Angle = FMath::FRandRange(0.f, 2.f * PI);
    const float Radius = FMath::FRandRange(100.f, SpawnRadius);

    const FVector SpawnOffset = FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.f);
    const FVector SpawnLocation = GetActorLocation() + SpawnOffset;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AForestAnt* NewAnt = GetWorld()->SpawnActor<AForestAnt>(AntClass, SpawnLocation, FRotator::ZeroRotator, Params);
    if (NewAnt)
    {
        NewAnt->RegisterWithQueen(this);
        TrackingArray.Add(NewAnt);
    }
}

void AAntQueen::PurgeDeadEntries()
{
    AliveWorkers.RemoveAll( [](const TWeakObjectPtr<AForestAnt>& P){ return !P.IsValid(); });
    AliveSoldiers.RemoveAll([](const TWeakObjectPtr<AForestAnt>& P){ return !P.IsValid(); });
}

void AAntQueen::OnAntDied(ABaseEnemy* DeadAnt)
{
    AForestAnt* DeadForestAnt = Cast<AForestAnt>(DeadAnt);
    if (!DeadForestAnt) return;

    AliveWorkers.RemoveAll( [DeadForestAnt](const TWeakObjectPtr<AForestAnt>& P){ return P.Get() == DeadForestAnt; });
    AliveSoldiers.RemoveAll([DeadForestAnt](const TWeakObjectPtr<AForestAnt>& P){ return P.Get() == DeadForestAnt; });
}

void AAntQueen::OnQueenDamaged(float NewHealth, float MaxHealth)
{
    // Emergency: immediately spawn soldiers when the queen takes a hit
    int32 ToSpawn = FMath::Min(EmergencySoldierCount, MaxSoldiers - AliveSoldiers.Num());
    for (int32 i = 0; i < ToSpawn; ++i)
    {
        SpawnAnt(SoldierAntClass, AliveSoldiers);
    }
}
