#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_DeliverFood.generated.h"

UCLASS()
class CRASHED_API UBTTask_DeliverFood : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_DeliverFood();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
											uint8* NodeMemory) override;
};
