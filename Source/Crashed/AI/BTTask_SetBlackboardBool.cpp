#include "BTTask_SetBlackboardBool.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_SetBlackboardBool::UBTTask_SetBlackboardBool()
{
	NodeName = "Set Blackboard Bool";
}

EBTNodeResult::Type UBTTask_SetBlackboardBool::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
															uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	BB->SetValueAsBool(KeyName, Value);
	return EBTNodeResult::Succeeded;
}
