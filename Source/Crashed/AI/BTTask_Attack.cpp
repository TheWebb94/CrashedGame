#include "BTTask_Attack.h"
#include "AIController.h"
#include "Crashed/NPC/BaseEnemy.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = "Attack";
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
												 uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller) return EBTNodeResult::Failed;

	ABaseEnemy* Enemy = Cast<ABaseEnemy>(Controller->GetPawn());
	if (!Enemy) return EBTNodeResult::Failed;

	Enemy->PerformAttack();
	return EBTNodeResult::Succeeded;
}
