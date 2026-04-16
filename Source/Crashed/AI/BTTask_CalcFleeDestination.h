#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_CalcFleeDestination.generated.h"

UCLASS()
class CRASHED_API UBTTask_CalcFleeDestination : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_CalcFleeDestination();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
											uint8* NodeMemory) override;
protected:
	// How far to run in the direction away from the player
	UPROPERTY(EditAnywhere, Category = "Flee")
	float FleeDistance = 600.f;
};
