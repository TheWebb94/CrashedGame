#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SetBlackboardBool.generated.h"

UCLASS()
class CRASHED_API UBTTask_SetBlackboardBool : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SetBlackboardBool();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
											uint8* NodeMemory) override;
protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName KeyName;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	bool Value = false;
};
