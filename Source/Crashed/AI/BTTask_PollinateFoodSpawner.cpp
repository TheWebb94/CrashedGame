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
	//config
	AAIController* Controller = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB  = OwnerComp.GetBlackboardComponent();
	
	//cast guards
	if (!Controller || !BB) return EBTNodeResult::Failed;

	ABee* Bee = Cast<ABee>(Controller->GetPawn());
	if (!Bee) return EBTNodeResult::Failed;

	AFoodSpawner* Spawner = Cast<AFoodSpawner>(BB->GetValueAsObject(TEXT("PollinateTarget")));
	
	//if the target is valid set it as the target for the bee
	if (!IsValid(Spawner))
	{
		BB->ClearValue(TEXT("PollinateTarget"));
		BB->SetValueAsBool(TEXT("HasPollinateTarget"), false);
		return EBTNodeResult::Failed;
	}

	//if the target is out of range, return early
	if (FVector::Dist(Bee->GetActorLocation(), Spawner->GetActorLocation()) > PollinateRange)
		return EBTNodeResult::Failed;

	//pollinate the foodspawner - allows spawning of food for x duration
	Spawner->Pollinate();

	//once compleete set the tree as pollinated in the bees tracking list to prevent repollination
	const float Expiry = GetWorld()->GetTimeSeconds() + Bee->PollinationCooldown;
	Bee->PollinationCooldowns.Add(TWeakObjectPtr<AFoodSpawner>(Spawner), Expiry);

	//then clear the pollination target
	BB->ClearValue(TEXT("PollinateTarget"));
	BB->SetValueAsBool(TEXT("HasPollinateTarget"), false);

	return EBTNodeResult::Succeeded;
}