#include "ForestAnt.h"
#include "AntQueen.h"
#include "AntHive.h"
#include "Crashed/HealthComponent.h"
#include "Crashed/NPC/EnemyAIController.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AForestAnt::AForestAnt()
{
    // Ants use the custom AI controller so return-home logic can override BT targets
    AIControllerClass = AEnemyAIController::StaticClass();
}

void AForestAnt::BeginPlay()
{
    Super::BeginPlay();
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

