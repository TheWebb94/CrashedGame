#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_DetectPlayer.generated.h"

UCLASS()
class CRASHED_API UBTService_DetectPlayer : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_DetectPlayer();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp,
						  uint8* NodeMemory, float DeltaSeconds) override;

	//Array of all acotrs the ants should flee from
	UPROPERTY(EditAnywhere, Category = "Flee")
	TArray<TSubclassOf<AActor>> ThreatClasses;

	// Ant starts fleeing when a threat enters this range
	UPROPERTY(EditAnywhere, Category = "Flee")
	float FleeRadius = 600.f;

	// Ant stops fleeing once all threats are this far away (must be > FleeRadius)
	UPROPERTY(EditAnywhere, Category = "Flee")
	float SafeRadius = 1000.f;
};
