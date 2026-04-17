#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_FindNearbyInjuredAlly.generated.h"

UCLASS()
class CRASHED_API UBTService_FindNearbyInjuredAlly : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_FindNearbyInjuredAlly();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	// Radius to scan for injured allies
	UPROPERTY(EditAnywhere, Category = "Healing")
	float ScanRadius = 800.f;

	// Ants below this health fraction are considered injured (1.0 = any damage at all)
	UPROPERTY(EditAnywhere, Category = "Healing")
	float HealthThreshold = 0.99f;
};
