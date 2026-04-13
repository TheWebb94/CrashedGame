#include "ForestAnt.h"
#include "AntQueen.h"
#include "AntHive.h"
#include "AAntAIController.h"
#include "Crashed/HealthComponent.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AForestAnt::AForestAnt()
{
    // Ants use the custom AI controller so return-home logic can override BT targets
    AIControllerClass = AAntAIController::StaticClass();
}

void AForestAnt::BeginPlay()
{
    Super::BeginPlay();

    // Set per-type return threshold (can be overridden per Blueprint child)
    switch (AntType)
    {
    case EAntType::Worker:  ReturnHealthThreshold = 0.80f; break;
    case EAntType::Soldier: ReturnHealthThreshold = 0.25f; break;
    case EAntType::Healer:  ReturnHealthThreshold = 0.00f; break; // healers never return home
    }

    if (HealthComponent)
    {
        HealthComponent->OnHealthChanged.AddDynamic(this, &AForestAnt::OnAntHealthChanged);
    }

    // Healers pulse a heal aura on a repeating timer
    if (AntType == EAntType::Healer && HealInterval > 0.f)
    {
        GetWorldTimerManager().SetTimer(HealTimerHandle, this, &AForestAnt::HealNearbyAnts, HealInterval, true, HealInterval);
    }
}

void AForestAnt::RegisterWithQueen(AAntQueen* InQueen)
{
    Queen = InQueen;
    if (InQueen)
    {
        OnEnemyDeath.AddDynamic(InQueen, &AAntQueen::OnAntDied);
    }
}

void AForestAnt::SetHomeHive(AAntHive* InHive)
{
    HomeHive = InHive;
}

void AForestAnt::OnDeath_Implementation()
{
    GetWorldTimerManager().ClearTimer(HealTimerHandle);
    Super::OnDeath_Implementation();
}

void AForestAnt::PerformAttack_Implementation()
{
    if (AntType == EAntType::Healer) return;

    APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!Player) return;

    if (FVector::DistSquared(GetActorLocation(), Player->GetActorLocation())
            > AttackRange * AttackRange) return;

    if (UHealthComponent* HC = Player->FindComponentByClass<UHealthComponent>())
    {
        HC->ApplyDamage(AttackDamage);
    }
}

void AForestAnt::OnAntHealthChanged(float NewHealth, float MaxHealth)
{
    if (bReturningHome) return;                    
    if (!HomeHive.IsValid()) return;              
    if (ReturnHealthThreshold <= 0.f) return;       
    if (MaxHealth <= 0.f) return;

    if ((NewHealth / MaxHealth) < ReturnHealthThreshold)
    {
        bReturningHome = true;
        RestoredDetectionRadius = DetectionRadius;
        DetectionRadius = 0.f; 
    }
}

void AForestAnt::CompleteHealing()
{
    bReturningHome = false;
    DetectionRadius = RestoredDetectionRadius;
}

void AForestAnt::HealNearbyAnts()
{
    // Find all AForestAnt actors and heal those within HealRadius
    TArray<AActor*> AllAnts;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AForestAnt::StaticClass(), AllAnts);

    const float RadiusSq = HealRadius * HealRadius;

    for (AActor* Actor : AllAnts)
    {
        if (Actor == this) continue;

        AForestAnt* Ant = Cast<AForestAnt>(Actor);
        if (!Ant || !Ant->HealthComponent) continue;

        if (FVector::DistSquared(GetActorLocation(), Ant->GetActorLocation()) <= RadiusSq)
        {
            Ant->HealthComponent->ApplyHealth(HealAmount);
        }
    }
}