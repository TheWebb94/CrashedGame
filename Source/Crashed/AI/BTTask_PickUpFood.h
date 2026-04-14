#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_PickUpFood.generated.h"

UCLASS()
class CRASHED_API UBTTask_PickUpFood : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_PickUpFood();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
											uint8* NodeMemory) override;
};
