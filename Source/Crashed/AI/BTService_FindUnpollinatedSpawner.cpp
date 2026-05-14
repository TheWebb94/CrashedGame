#include "BTService_FindUnpollinatedSpawner.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Crashed/Items/FoodSpawner.h"
#include "Crashed/NPC/Bee/Bee.h"


UBTService_FindUnpollinatedSpawner::UBTService_FindUnpollinatedSpawner()
{
    NodeName = TEXT("FindUnpollinatedSpawner");
    Interval = 1.0f;
    RandomDeviation = 0.2f;
}

void UBTService_FindUnpollinatedSpawner::TickNode(UBehaviorTreeComponent& OwnerComp,
                                                   uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* Controller = OwnerComp.GetAIOwner();
    UBlackboardComponent* BB  = OwnerComp.GetBlackboardComponent();
    if (!Controller || !BB) return;

    ABee* Bee = Cast<ABee>(Controller->GetPawn());
    if (!Bee || Bee->bIsDefending || !Bee->bIsSwarmMaster) return;

    const FVector BeeLocation = Bee->GetActorLocation();
    const float   CurrentTime = GetWorld()->GetTimeSeconds();
    const float   RadiusSq    = Bee->PollinationScanRadius * Bee->PollinationScanRadius;

    TArray<AActor*> AllSpawners;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFoodSpawner::StaticClass(), AllSpawners);

    AFoodSpawner* Best      = nullptr;
    float         BestDistSq = RadiusSq;

    for (AActor* Actor : AllSpawners)
    {
        AFoodSpawner* Spawner = Cast<AFoodSpawner>(Actor);
        if (!Spawner || !Spawner->RequiresPollination()) continue;
        if (Spawner->IsPollinated()) continue;

        TWeakObjectPtr<AFoodSpawner> SpawnerPtr(Spawner);
        if (const float* Expiry = Bee->PollinationCooldowns.Find(SpawnerPtr))
        {
            if (CurrentTime < *Expiry) continue;
        }

        const float DistSq = FVector::DistSquared(BeeLocation, Spawner->GetActorLocation());
        if (DistSq < BestDistSq) { BestDistSq = DistSq; Best = Spawner; }
    }

    if (Best)
    {
        BB->SetValueAsObject(TEXT("PollinateTarget"), Best);
        BB->SetValueAsBool(TEXT("HasPollinateTarget"), true);
    }
    else
    {
        BB->ClearValue(TEXT("PollinateTarget"));
        BB->SetValueAsBool(TEXT("HasPollinateTarget"), false);
    }
}