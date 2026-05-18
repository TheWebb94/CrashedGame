#include "BeeHive.h"
#include "Bee.h"
#include "BeeAIController.h"
#include "Components/StaticMeshComponent.h"
#include "Crashed/HealthComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/World.h"

ABeeHive::ABeeHive() //constructor config
{
    PrimaryActorTick.bCanEverTick = false;
    
    //set mesh
    HiveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HiveMesh"));
    RootComponent = HiveMesh;
    
    //set health component
    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void ABeeHive::BeginPlay()
{
    Super::BeginPlay();

    
    if (HealthComponent)
        HealthComponent->OnHealthChanged.AddDynamic(this, &ABeeHive::OnHiveDamaged); //bind an event to health changed

    //spawn the initital swarm
    for (int32 i = 0; i < MaxBees; ++i)
        SpawnBee();
}

///<summary>
///if the swarm master becomes invalid, promote the next bee in the tracked array to swarm master
///</summary>
void ABeeHive::PromoteSwarmMaster()
{
    if (SwarmMaster.IsValid()) return;  //check validity

    for (auto& BeePtr : AliveBees) 
    {
        if (BeePtr.IsValid())
        {
            SwarmMaster = BeePtr;
            BeePtr->bIsSwarmMaster = true; //iterate through bees and promote to master
            return;
        }
    }
}

///Central method for spawning bees
void ABeeHive::SpawnBee()
{
    if (!BeeClass) return; //valid class not set in editor guard

    AliveBees.RemoveAll([](const TWeakObjectPtr<ABee>& P) { return !P.IsValid(); }); //remove all bees that are not valid from the list
    if (AliveBees.Num() >= MaxBees) return; //if we are at maximum number of bees do nothing

    //find a nearby spawn location
    const float Angle  = FMath::FRandRange(0.f, 2.f * PI);
    const float Radius = FMath::FRandRange(100.f, SpawnRadius);
    
    // setup the location
    const FVector Loc = GetActorLocation()
        + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 150.f);

    //setup the collision for safe spawning
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    //then spawn the bee
    ABee* NewBee = GetWorld()->SpawnActor<ABee>(BeeClass, Loc, FRotator::ZeroRotator, Params);
    if (!NewBee) return; //safety check before continuing

    //then setup values on the spawned bee
    NewBee->OwnerHive = this;
    NewBee->OnEnemyDeath.AddDynamic(this, &ABeeHive::OnBeeDied);
    AliveBees.Add(NewBee); //add to tracking array
    
    // Inherit defend state from swarm master if a fight is already in progress
    if (SwarmMaster.IsValid() && SwarmMaster->bIsDefending && SwarmMaster->CurrentAttackTarget)
    {
        NewBee->bIsDefending = true;
        NewBee->CurrentAttackTarget = SwarmMaster->CurrentAttackTarget;

        if (ABeeAIController* AIC = Cast<ABeeAIController>(NewBee->GetController()))
        {
            if (AIC->BBC)
            {
                AIC->BBC->SetValueAsObject(TEXT("TargetActor"), SwarmMaster->CurrentAttackTarget);
                AIC->BBC->SetValueAsBool(TEXT("IsDefending"), true);
                AIC->BBC->SetValueAsBool(TEXT("HasLineOfSight"), true);
            }
        }
    }
    
    PromoteSwarmMaster(); //check the swarm for swarm master and promote one of ot valid
}

//TODO: add instigator for damage so the real target is known -  low priority, in current build no other NPCs target the beehive but could be implemented in the future
void ABeeHive::OnHiveDamaged(float NewHealth, float MaxHealth)
{
    // Fall back to the player pawn as the swarm target
    AActor* Target = GetWorld()->GetFirstPlayerController()
        ? GetWorld()->GetFirstPlayerController()->GetPawn()
        : nullptr;

    SendBeesToAttack(Target); //bees store the given target and all attack
}

void ABeeHive::SendBeesToAttack(AActor* Target)
{
    if (!Target) return; //if no target, no behaviour

    AliveBees.RemoveAll([](const TWeakObjectPtr<ABee>& P) { return !P.IsValid(); }); //remove invalid bees from array

    for (auto& BeePtr : AliveBees) //loop through all bees and set their state to defending, and give them a  target
    {
        if (!BeePtr.IsValid()) continue;
        ABee* Bee = BeePtr.Get();
        Bee->bIsDefending = true;
        Bee->CurrentAttackTarget = Target;

        if (ABeeAIController* AIC = Cast<ABeeAIController>(Bee->GetController())) //set these values in the BB
        {
            if (AIC->BBC)
            {
                AIC->BBC->SetValueAsObject(TEXT("TargetActor"), Target);
                AIC->BBC->SetValueAsBool(TEXT("IsDefending"), true);
                AIC->BBC->SetValueAsBool(TEXT("HasLineOfSight"), true);
            }
        }
    }

    // Auto-reset defend state after DefendDuration seconds
    GetWorldTimerManager().SetTimer(DefendResetTimerHandle,
        this, &ABeeHive::ResetDefendState, DefendDuration, false);
}

void ABeeHive::ResetDefendState()
{
    AliveBees.RemoveAll([](const TWeakObjectPtr<ABee>& P) { return !P.IsValid(); });

    // If any bee still has a living target, keep defending and re-check later
    for (auto& BeePtr : AliveBees)
    {
        //guards + config
        if (!BeePtr.IsValid()) continue;
        AActor* Target = BeePtr->CurrentAttackTarget;
        if (!Target) continue;

        UHealthComponent* HC = Target->FindComponentByClass<UHealthComponent>();
        if (HC && !HC->IsDead())
        {
            //set timer for the reset of defend state
            GetWorldTimerManager().SetTimer(DefendResetTimerHandle,this, &ABeeHive::ResetDefendState, DefendDuration, false);
            return;
        }
    }

    // Target is dead or gone — stand down
    for (auto& BeePtr : AliveBees)
    {
        if (!BeePtr.IsValid()) continue;
        ABee* Bee = BeePtr.Get();
        Bee->bIsDefending = false;
        Bee->CurrentAttackTarget = nullptr;

        if (ABeeAIController* AIC = Cast<ABeeAIController>(Bee->GetController()))
        {
            if (AIC->BBC)
            {
                AIC->BBC->SetValueAsBool(TEXT("IsDefending"), false);
                AIC->BBC->SetValueAsBool(TEXT("HasLineOfSight"), false);
                AIC->BBC->ClearValue(TEXT("TargetActor"));
            }
        }
    }
}

//When a bee dies, remove form tracking array, and add a pending respawn
void ABeeHive::OnBeeDied(ABaseEnemy* DeadBee)
{
    AliveBees.RemoveAll([DeadBee](const TWeakObjectPtr<ABee>& P)
    {
        return !P.IsValid() || P.Get() == Cast<ABee>(DeadBee);
    });

    ++PendingRespawns;
    FTimerHandle TempHandle;
    GetWorldTimerManager().SetTimer(TempHandle, this,
        &ABeeHive::TryRespawnBee, BeeRespawnDelay, false);
}

///if the hive has available space in the swarm (>0 pending respawns) spawn a bee
void ABeeHive::TryRespawnBee()
{
    if (PendingRespawns <= 0) return;
    --PendingRespawns;
    SpawnBee();
}