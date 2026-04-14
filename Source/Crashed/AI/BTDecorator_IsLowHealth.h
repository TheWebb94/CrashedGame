// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_IsLowHealth.generated.h"

/**
 * 
 */
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
	float HealthThreshold = 0.8f;
};
