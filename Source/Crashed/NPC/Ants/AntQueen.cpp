#include "AntQueen.h"
#include "ForestAnt.h"
#include "AntHive.h"
#include "Crashed/HealthComponent.h"
#include "Crashed/NPC/PatrolPath.h"
#include "Engine/World.h"

AAntQueen::AAntQueen() //constructor for setting up values
{
    DetectionRadius = 0.f; //the queen never has any perception behaviour implemented, her responsibility is to blindly create ants
}

//simply increase food amount by the amount given
void AAntQueen::ReceiveFood(float Amount)
{
    FoodSupply += Amount;
}

void AAntQueen::BeginPlay()
{
    Super::BeginPlay();

    //bind an event to the queen taking damage
    if (HealthComponent)
        HealthComponent->OnHealthChanged.AddDynamic(this, &AAntQueen::OnQueenDamaged);

    //set the spawn timer for spawned ants
    GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AAntQueen::CheckAndSpawn,
                                    SpawnInterval, true, SpawnInterval);
    //spawn the initial healers, not factored in with the food cost asssociated with spawning
    SpawnInitialHealers();
}


void AAntQueen::CheckAndSpawn()
{
    //cleean out any invalid (dead) ants from the array of alive ants
    //allows proper workers : soldiers ratio to be maintained 
    PurgeDeadEntries();

    //resets the lists
    const int32 NumWorkers  = AliveWorkers.Num();
    const int32 NumSoldiers = AliveSoldiers.Num();

    //as the ratio is workers / soldiers, i set to spawn a soldier here to avoid DIV/0 case
    if (NumSoldiers == 0)
    {
        SpawnAnt(SoldierAntClass, AliveSoldiers);
    }

    //rational whole number now able to be evaluated  
    const float CurrentRatio = (float)NumWorkers / (float)NumSoldiers;

    //conditions for soldier spawn = 0 current soldiers
    //                              current soldier count < maximum soldier count
    //                              we are below desired ration of soldiers : workers
    if (NumSoldiers < MaxSoldiers && CurrentRatio >= WorkerToSoldierRatio)
    {
        SpawnAnt(SoldierAntClass, AliveSoldiers);
    }
    else if (NumWorkers < MaxWorkers) //if we are above the desired ratio of soldiers : workers, spawn worker
    {
        if (AliveWorkers.Num() < MinWorkersBeforeFoodCost) // we always want a minimum amount of workers to be spawned for free to prevent colony collapse
            SpawnAnt(WorkerAntClass, AliveWorkers);
        else if (FoodSupply >= FoodCostPerSpawn) //after minimum is reached, evaluate food supply first 
        {
            FoodSupply -= FoodCostPerSpawn;
            SpawnAnt(WorkerAntClass, AliveWorkers);
        }
    }
}

void AAntQueen::SpawnAnt(TSubclassOf<AForestAnt> AntClass, TArray<TWeakObjectPtr<AForestAnt>>& TrackingArray)
{
    //config guard
    if (!AntClass) return;

    //find a random location near to the queen
    const float  Angle         = FMath::FRandRange(0.f, 2.f * PI);
    const float  Radius        = FMath::FRandRange(100.f, SpawnRadius);
    const FVector SpawnLocation = GetActorLocation() + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);
    
    //helps spawn the ants in a safe way to prevent collision bugs
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    //spawn the ant
    AForestAnt* NewAnt = GetWorld()->SpawnActor<AForestAnt>(AntClass, SpawnLocation,FRotator::ZeroRotator, Params);
   
    //cast guard
    if (!NewAnt) return;

    //configre the settings on the new ant so it is tracked properly
    NewAnt->RegisterWithQueen(this);
    NewAnt->SetHomeHive(HomeHive);

    if (NewAnt->AntType == EAntType::Worker)
    {
        // Workers always follow the hive patrol path
        NewAnt->PatrolPath = AntPatrolPath;
    }
    else if (NewAnt->AntType == EAntType::Soldier)
    {
        // Randomly assign patrol style based on ratio
        const ESoldierPatrolType PatrolType = (FMath::FRand() < PathPatrolSoldierRatio)
            ? ESoldierPatrolType::PathPatrol //if below the ratio, add to the path patrol (accompanies workker patrol)
            : ESoldierPatrolType::RandomPatrol; //otherwise randomly patrol

        NewAnt->SetSoldierPatrolType(PatrolType);

        if (PatrolType == ESoldierPatrolType::PathPatrol) //gives the path to the soldier
            NewAnt->PatrolPath = AntPatrolPath;
    }

    TrackingArray.Add(NewAnt);
}

//modular method to clean out the tracking arrays
void AAntQueen::PurgeDeadEntries()
{
    AliveWorkers.RemoveAll( [](const TWeakObjectPtr<AForestAnt>& P){ return !P.IsValid(); });
    AliveSoldiers.RemoveAll([](const TWeakObjectPtr<AForestAnt>& P){ return !P.IsValid(); });
    AliveHealers.RemoveAll( [](const TWeakObjectPtr<AForestAnt>& P){ return !P.IsValid(); });
}

//behaviour for what to do when an ant is killed
void AAntQueen::OnAntDied(ABaseEnemy* DeadAnt)
{
    //config
    AForestAnt* Dead = Cast<AForestAnt>(DeadAnt);
    
    //validity check
    if (!Dead) return;

    //look through tracking arrays to check if the dead ant is in there, if it is remove from array
    AliveWorkers.RemoveAll( [Dead](const TWeakObjectPtr<AForestAnt>& P){ return P.Get() == Dead; });
    AliveSoldiers.RemoveAll([Dead](const TWeakObjectPtr<AForestAnt>& P){ return P.Get() == Dead; });

    
    const bool bWasHealer = AliveHealers.ContainsByPredicate(
        [Dead](const TWeakObjectPtr<AForestAnt>& P){ return P.Get() == Dead; });

    if (bWasHealer) //if it was a healer, sachedule another for special spawning, again will help rpevent colony collapse
    {
        AliveHealers.RemoveAll([Dead](const TWeakObjectPtr<AForestAnt>& P){ return P.Get() == Dead; });
        ScheduleHealerReplacement();
    }
}

///<summary>Pass in an aggressor to notify all soldiers tracked by thee queen to target the threat</summary>
///<param name="Target">Aggressor to the hive</param>
void AAntQueen::AlertSoldiersNearby(FVector Origin, AActor* Target, float Radius)
{
    //config
    PurgeDeadEntries();
    const float RadiusSq = Radius * Radius;
    
    for (auto& SoldierPtr : AliveSoldiers) //look for all soldiers in the tracking array, and set their current target to an aggressor to the hive (target parameter)
    {
        if (!SoldierPtr.IsValid()) continue;
        if (FVector::DistSquared(Origin, SoldierPtr->GetActorLocation()) <= RadiusSq) //if close enough
            SoldierPtr->SetAttackTarget(Target); //set target
    }
}

//loops through all soldiers in the hive and set the defend flag 
void AAntQueen::NotifyAllSoldiersDefend()
{
    PurgeDeadEntries();
    for (auto& SoldierPtr : AliveSoldiers)
    {
        if (SoldierPtr.IsValid())
            SoldierPtr->SetDefendHive(true);
    }
}

//spawns all initial healers
void AAntQueen::SpawnInitialHealers()
{
    for (int32 i = 0; i < InitialHealerCount; ++i)
        SpawnAnt(HealerAntClass, AliveHealers);
}

//when a healer dies, 
void AAntQueen::ScheduleHealerReplacement()
{
    FTimerHandle TempHandle;
    GetWorldTimerManager().SetTimer(TempHandle, this,
                                    &AAntQueen::SpawnReplacementHealer, HealerReplaceDelay, false);
}

void AAntQueen::SpawnReplacementHealer()
{
    SpawnAnt(HealerAntClass, AliveHealers);
}

///<summary>
///behaviour for when the queen takesa damage
///</summary>
void AAntQueen::OnQueenDamaged(float NewHealth, float MaxHealth)
{
    if (MaxHealth <= 0.f) return;

    // Any damage to the queen immediately pulls all soldiers back to defend
    NotifyAllSoldiersDefend();

    const float Pct = NewHealth / MaxHealth;

    //then at specific health thresholds, spawn a free wave of defensive troops
    if (!bThreshold66Triggered && Pct <= 0.66f)
    {
        bThreshold66Triggered = true;
        for (int32 i = 0; i < SoldiersSpawnedAt66; ++i)
            SpawnAnt(SoldierAntClass, AliveSoldiers);
        for (int32 i = 0; i < 3; ++i)
            SpawnAnt(HealerAntClass, AliveHealers);
    }

    if (!bThreshold33Triggered && Pct <= 0.33f)
    {
        bThreshold33Triggered = true;
        for (int32 i = 0; i < SoldiersSpawnedAt33; ++i)
            SpawnAnt(SoldierAntClass, AliveSoldiers);
        for (int32 i = 0; i < 3; ++i)
            SpawnAnt(HealerAntClass, AliveHealers);
    }
}
