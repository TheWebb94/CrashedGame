// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_FindNearbyFood.generated.h"

/**
 * 
 */
UCLASS()
class CRASHED_API UBTService_FindNearbyFood : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_FindNearbyFood();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp,
						  uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Food Sensing")
	float ScanRadius = 800.f;
};
