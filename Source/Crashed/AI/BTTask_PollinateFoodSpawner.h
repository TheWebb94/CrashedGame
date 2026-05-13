#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_PollinateFoodSpawner.generated.h"

UCLASS()
class CRASHED_API UBTTask_PollinateFoodSpawner : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_PollinateFoodSpawner();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
											uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Pollination")
	float PollinateRange = 200.f;
};