#pragma once

#include "CoreMinimal.h"
#include "Crashed/NPC/BaseEnemy.h"
#include "ForestBee.generated.h"

class ABeeHive;

UCLASS(Abstract)
class CRASHED_API AForestBee : public ABaseEnemy
{
	GENERATED_BODY()

public:
	// Called by the hive when it has been attacked — makes this bee aggressive
	void ActivateSwarmMode();

	void SetOwningHive(ABeeHive* InHive);

	UPROPERTY(BlueprintReadOnly, Category = "Bee")
	bool bIsAngry = false;

protected:
	// How large the detection radius becomes in swarm mode
	UPROPERTY(EditDefaultsOnly, Category = "Bee")
	float AngryDetectionRadius = 3000.f;

	virtual void OnDeath_Implementation() override;

private:
	TWeakObjectPtr<ABeeHive> OwningHive;
};
