#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_BeeFlocking.generated.h"

UCLASS()
class CRASHED_API UBTService_BeeFlocking : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_BeeFlocking();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp,
						  uint8* NodeMemory, float DeltaSeconds) override;
};