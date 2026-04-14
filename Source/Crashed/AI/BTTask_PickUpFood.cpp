#include "BTTask_PickUpFood.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Crashed/Items/Food.h"
#include "Crashed/NPC/Ants/ForestAnt.h"

UBTTask_PickUpFood::UBTTask_PickUpFood()
{
	NodeName = "PickUpFood";
}

EBTNodeResult::Type UBTTask_PickUpFood::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
													uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!BB || !Controller)
		return EBTNodeResult::Failed;

	AFood* Food = Cast<AFood>(BB->GetValueAsObject(TEXT("FoodTarget")));
	if (!Food || !Food->IsAvailable())
	{
		BB->SetValueAsObject(TEXT("FoodTarget"), nullptr);
		BB->SetValueAsBool(TEXT("HasFoodTarget"), false);
		return EBTNodeResult::Failed;
	}

	const float Amount = Food->TakeFood();

	AForestAnt* Ant = Cast<AForestAnt>(Controller->GetPawn());
	if (Ant)
		Ant->SetCarryingFood(true, Amount);

	BB->SetValueAsBool(TEXT("IsCarryingFood"), true);
	BB->SetValueAsObject(TEXT("FoodTarget"), nullptr);
	BB->SetValueAsBool(TEXT("HasFoodTarget"), false);

	return EBTNodeResult::Succeeded;
}
