// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_GetRandomPatrolPoint.generated.h"

/**
 * 
 */
UCLASS()
class CRASHED_API UBTTask_GetRandomPatrolPoint : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_GetRandomPatrolPoint();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	protected:
	//Radius from home to patrol
	UPROPERTY(EditAnywhere, Category = "Patrol")
	float PatrolRadius = 600.f;
};
