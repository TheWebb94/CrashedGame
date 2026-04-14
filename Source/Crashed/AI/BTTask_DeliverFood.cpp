#include "BTTask_DeliverFood.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Crashed/NPC/Ants/AntHive.h"
#include "Crashed/NPC/Ants/AntQueen.h"
#include "Crashed/NPC/Ants/ForestAnt.h"

UBTTask_DeliverFood::UBTTask_DeliverFood()
{
	NodeName = "DeliverFood";
}

EBTNodeResult::Type UBTTask_DeliverFood::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
													 uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!BB || !Controller)
		return EBTNodeResult::Failed;

	AForestAnt* Ant = Cast<AForestAnt>(Controller->GetPawn());
	if (!Ant || !Ant->HomeHive.IsValid())
		return EBTNodeResult::Failed;

	AAntQueen* Queen = Ant->HomeHive->Queen;
	if (Queen)
		Queen->ReceiveFood(Ant->GetCarriedFoodAmount());

	Ant->SetCarryingFood(false);
	BB->SetValueAsBool(TEXT("IsCarryingFood"), false);

	return EBTNodeResult::Succeeded;
}
