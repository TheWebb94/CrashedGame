#include "BTTask_PollinateFoodSpawner.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Crashed/Items/FoodSpawner.h"
#include "Crashed/NPC/Bee/Bee.h"
#include "Engine/World.h"

UBTTask_PollinateFoodSpawner::UBTTask_PollinateFoodSpawner()
{
	NodeName = TEXT("PollinateFoodSpawner");
}

EBTNodeResult::Type UBTTask_PollinateFoodSpawner::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB  = OwnerComp.GetBlackboardComponent();
	if (!Controller || !BB) return EBTNodeResult::Failed;

	ABee* Bee = Cast<ABee>(Controller->GetPawn());
	if (!Bee) return EBTNodeResult::Failed;

	AFoodSpawner* Spawner = Cast<AFoodSpawner>(BB->GetValueAsObject(TEXT("PollinateTarget")));
	if (!IsValid(Spawner))
	{
		BB->ClearValue(TEXT("PollinateTarget"));
		BB->SetValueAsBool(TEXT("HasPollinateTarget"), false);
		return EBTNodeResult::Failed;
	}

	if (FVector::Dist(Bee->GetActorLocation(), Spawner->GetActorLocation()) > PollinateRange)
		return EBTNodeResult::Failed;

	Spawner->Pollinate();

	const float Expiry = GetWorld()->GetTimeSeconds() + Bee->PollinationCooldown;
	Bee->PollinationCooldowns.Add(TWeakObjectPtr<AFoodSpawner>(Spawner), Expiry);

	BB->ClearValue(TEXT("PollinateTarget"));
	BB->SetValueAsBool(TEXT("HasPollinateTarget"), false);

	return EBTNodeResult::Succeeded;
}