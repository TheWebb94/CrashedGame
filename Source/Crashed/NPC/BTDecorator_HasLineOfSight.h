// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "BTDecorator_HasLineOfSight.generated.h"

/**
 * 
 */
UCLASS()
class CRASHED_API UBTDecorator_HasLineOfSight : public UBTDecorator_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTDecorator_HasLineOfSight();
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,	uint8* NodeMemory) const override;

};
