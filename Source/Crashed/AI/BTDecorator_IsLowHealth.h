#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_IsLowHealth.generated.h"

class UHealthComponent;

UCLASS()
class CRASHED_API UBTDecorator_IsLowHealth : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_IsLowHealth();
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
											uint8* NodeMemory) const override;

protected:
	UPROPERTY(EditAnywhere, Category="Condition", meta=(ClampMin="0", ClampMax="1"))
	float HealthThreshold = 0.3f;

	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	UFUNCTION()
	void OnHealthChanged(float NewHealth, float MaxHealth);

	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
	TWeakObjectPtr<UHealthComponent>       CachedHealthComp;
};
