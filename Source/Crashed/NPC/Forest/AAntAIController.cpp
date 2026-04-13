#include "AAntAIController.h"
#include "ForestAnt.h"
#include "AntHive.h"
#include "Crashed/NPC/BaseEnemy.h"
#include "Crashed/NPC/EnemyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

void AAntAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);   

    // Cache the hive world position once so patrol tasks can read it any time
    if (AForestAnt* Ant = Cast<AForestAnt>(InPawn))
    {
        if (Ant->HomeHive.IsValid() && GetBlackboardComponent())
        {
            GetBlackboardComponent()->SetValueAsVector(
                AEnemyAIController::BB_HiveLocation,
                Ant->HomeHive->GetActorLocation());
        }
    }
}

void AAntAIController::UpdateBlackboard()
{
    AForestAnt* Ant = Cast<AForestAnt>(GetPawn());
    if (!Ant)
    {
        Super::UpdateBlackboard();
        return;
    }

    UBlackboardComponent* BB = GetBlackboardComponent();
    if (!BB) return;

    // Keep the BT informed of the C++ return-home flag
    BB->SetValueAsBool(AEnemyAIController::BB_IsReturningHome, Ant->bReturningHome);

    // While retreating, point TargetLocation at the hive and clear any combat target
    if (Ant->bReturningHome && Ant->HomeHive.IsValid())
    {
        BB->ClearValue(AEnemyAIController::BB_TargetActor);
        BB->SetValueAsVector(AEnemyAIController::BB_TargetLocation,
                             Ant->HomeHive->GetActorLocation());
        return;
    }

    // --- Find nearest hostile within detection radius ---
    const FVector AntPos   = Ant->GetActorLocation();
    const float   RadiusSq = Ant->DetectionRadius * Ant->DetectionRadius;

    AActor* BestTarget  = nullptr;
    float   BestDistSq  = RadiusSq;   // nothing found beyond the radius

    // Player
    if (APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        if (IsValid(Player))
        {
            const float D = FVector::DistSquared(AntPos, Player->GetActorLocation());
            if (D <= BestDistSq) { BestDistSq = D; BestTarget = Player; }
        }
    }

    // Any other BaseEnemy that isn't part of the ant colony
    TArray<AActor*> AllEnemies;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseEnemy::StaticClass(), AllEnemies);
    for (AActor* Actor : AllEnemies)
    {
        if (!IsValid(Actor)) continue;
        if (Cast<AForestAnt>(Actor)) continue;   // same faction — skip

        const float D = FVector::DistSquared(AntPos, Actor->GetActorLocation());
        if (D < BestDistSq) { BestDistSq = D; BestTarget = Actor; }
    }

    if (BestTarget)
    {
        BB->SetValueAsObject(AEnemyAIController::BB_TargetActor, BestTarget);
        BB->SetValueAsVector(AEnemyAIController::BB_TargetLocation,
                             BestTarget->GetActorLocation());
    }
    else
    {
        BB->ClearValue(AEnemyAIController::BB_TargetActor);
        BB->ClearValue(AEnemyAIController::BB_TargetLocation);
    }
}