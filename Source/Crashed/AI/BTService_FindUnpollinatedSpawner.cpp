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

    //config
    AAIController* Controller = OwnerComp.GetAIOwner();
    UBlackboardComponent* BB  = OwnerComp.GetBlackboardComponent();
   
    //cast guards
    if (!Controller || !BB) return;

    ABee* Bee = Cast<ABee>(Controller->GetPawn());
    if (!Bee || Bee->bIsDefending || !Bee->bIsSwarmMaster) return;

    const FVector BeeLocation = Bee->GetActorLocation();
    const float   CurrentTime = GetWorld()->GetTimeSeconds();
    const float   RadiusSq    = Bee->PollinationScanRadius * Bee->PollinationScanRadius;

    //create tracking array of all spawners
    TArray<AActor*> AllSpawners;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFoodSpawner::StaticClass(), AllSpawners);

    AFoodSpawner* Best      = nullptr;
    float         BestDistSq = RadiusSq;

    //look through all foodspawners
    for (AActor* Actor : AllSpawners)
    {
        //cast guard, and validity checks
        AFoodSpawner* Spawner = Cast<AFoodSpawner>(Actor);
        if (!Spawner || !Spawner->RequiresPollination()) continue;
        if (Spawner->IsPollinated()) continue;

        //lookup the found foodspawner in the swarmmaster list of foodspawners to check if thye have pollinated this tree within the pollination cooldown timer
        TWeakObjectPtr<AFoodSpawner> SpawnerPtr(Spawner);
        if (const float* Expiry = Bee->PollinationCooldowns.Find(SpawnerPtr))
        {
            if (CurrentTime < *Expiry) continue;
        }

        //check through all valid, unpollinated foodspawners and deteermine the closest one
        const float DistSq = FVector::DistSquared(BeeLocation, Spawner->GetActorLocation());
        if (DistSq < BestDistSq) { BestDistSq = DistSq; Best = Spawner; }
    }

    //then set the evaluated foodswpaner as the target
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