#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_HealAlly.generated.h"

UCLASS()
class CRASHED_API UBTTask_HealAlly : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_HealAlly();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// HP restored per heal tick
	UPROPERTY(EditAnywhere, Category = "Healing")
	float HealAmount = 15.f;

	// Heal ticks per second
	UPROPERTY(EditAnywhere, Category = "Healing")
	float HealRate = 1.f;

	// Must be within this distance to heal (should match MoveTo AcceptRadius)
	UPROPERTY(EditAnywhere, Category = "Healing")
	float HealRange = 200.f;

private:
	FTimerHandle HealTimerHandle;
	UBehaviorTreeComponent* CachedOwnerComp = nullptr;

	void ClearBBKeys(UBlackboardComponent* BB);
	void DoHealTick();
};
