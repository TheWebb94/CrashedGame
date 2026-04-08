#include "AntHive.h"
#include "ForestAnt.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Crashed/HealthComponent.h"

AAntHive::AAntHive()
{
    PrimaryActorTick.bCanEverTick = false;

    HiveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HiveMesh"));
    RootComponent = HiveMesh;

    HealZone = CreateDefaultSubobject<USphereComponent>(TEXT("HealZone"));
    HealZone->SetupAttachment(RootComponent);
    HealZone->SetSphereRadius(600.f);
    HealZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    HealZone->SetCollisionResponseToAllChannels(ECR_Ignore);
    HealZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AAntHive::BeginPlay()
{
    Super::BeginPlay();

    HealZone->OnComponentBeginOverlap.AddDynamic(this, &AAntHive::OnHealZoneBeginOverlap);
    HealZone->OnComponentEndOverlap.AddDynamic(this, &AAntHive::OnHealZoneEndOverlap);

    GetWorldTimerManager().SetTimer(HealTickTimer, this, &AAntHive::HealTick, HealTickInterval, true, HealTickInterval);
}

void AAntHive::OnHealZoneBeginOverlap(UPrimitiveComponent*, AActor* OtherActor,
    UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    if (AForestAnt* Ant = Cast<AForestAnt>(OtherActor))
    {
        AntsInHealZone.AddUnique(Ant);
    }
}

void AAntHive::OnHealZoneEndOverlap(UPrimitiveComponent*, AActor* OtherActor,
    UPrimitiveComponent*, int32)
{
    if (AForestAnt* Ant = Cast<AForestAnt>(OtherActor))
    {
        AntsInHealZone.RemoveAll([Ant](const TWeakObjectPtr<AForestAnt>& P){ return P.Get() == Ant; });
    }
}

void AAntHive::HealTick()
{
    for (int32 i = AntsInHealZone.Num() - 1; i >= 0; --i)
    {
        AForestAnt* Ant = AntsInHealZone[i].Get();
        if (!IsValid(Ant))
        {
            AntsInHealZone.RemoveAt(i);
            continue;
        }

        if (!Ant->HealthComponent) continue;

        Ant->HealthComponent->ApplyHealth(HealTickAmount);

        // If the ant came home to heal and is now at full health, release it
        if (Ant->bReturningHome && Ant->HealthComponent->GetHealthPercent() >= 1.0f)
        {
            Ant->CompleteHealing();
        }
    }
}