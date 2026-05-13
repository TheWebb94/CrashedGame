#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_FindUnpollinatedSpawner.generated.h"

UCLASS()
class CRASHED_API UBTService_FindUnpollinatedSpawner : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_FindUnpollinatedSpawner();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp,
						  uint8* NodeMemory, float DeltaSeconds) override;
};