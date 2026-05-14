#include "BeeHive.h"
#include "Bee.h"
#include "BeeAIController.h"
#include "Components/StaticMeshComponent.h"
#include "Crashed/HealthComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/World.h"

ABeeHive::ABeeHive()
{
    PrimaryActorTick.bCanEverTick = false;

    HiveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HiveMesh"));
    RootComponent = HiveMesh;

    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void ABeeHive::BeginPlay()
{
    Super::BeginPlay();

    if (HealthComponent)
        HealthComponent->OnHealthChanged.AddDynamic(this, &ABeeHive::OnHiveDamaged);

    for (int32 i = 0; i < MaxBees; ++i)
        SpawnBee();
}

void ABeeHive::PromoteSwarmMaster()
{
    if (SwarmMaster.IsValid()) return;

    for (auto& BeePtr : AliveBees)
    {
        if (BeePtr.IsValid())
        {
            SwarmMaster = BeePtr;
            BeePtr->bIsSwarmMaster = true;
            return;
        }
    }
}

void ABeeHive::SpawnBee()
{
    if (!BeeClass) return;

    AliveBees.RemoveAll([](const TWeakObjectPtr<ABee>& P) { return !P.IsValid(); });
    if (AliveBees.Num() >= MaxBees) return;

    const float Angle  = FMath::FRandRange(0.f, 2.f * PI);
    const float Radius = FMath::FRandRange(100.f, SpawnRadius);
    
    // Spawn above hive so flying mode activates before physics settles
    const FVector Loc = GetActorLocation()
        + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 150.f);

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    ABee* NewBee = GetWorld()->SpawnActor<ABee>(BeeClass, Loc, FRotator::ZeroRotator, Params);
    if (!NewBee) return;

    NewBee->OwnerHive = this;
    NewBee->OnEnemyDeath.AddDynamic(this, &ABeeHive::OnBeeDied);
    AliveBees.Add(NewBee);
    
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
    
    PromoteSwarmMaster();
}

void ABeeHive::OnHiveDamaged(float NewHealth, float MaxHealth)
{
    // Fall back to the player pawn as the swarm target
    AActor* Target = GetWorld()->GetFirstPlayerController()
        ? GetWorld()->GetFirstPlayerController()->GetPawn()
        : nullptr;

    SendBeesToAttack(Target);
}

void ABeeHive::SendBeesToAttack(AActor* Target)
{
    if (!Target) return;

    AliveBees.RemoveAll([](const TWeakObjectPtr<ABee>& P) { return !P.IsValid(); });

    for (auto& BeePtr : AliveBees)
    {
        if (!BeePtr.IsValid()) continue;
        ABee* Bee = BeePtr.Get();
        Bee->bIsDefending = true;
        Bee->CurrentAttackTarget = Target;

        if (ABeeAIController* AIC = Cast<ABeeAIController>(Bee->GetController()))
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

void ABeeHive::TryRespawnBee()
{
    if (PendingRespawns <= 0) return;
    --PendingRespawns;
    SpawnBee();
}