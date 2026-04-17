// Fill out your copyright notice in the Description page of Project Settings.
#include "BTTask_GetRandomPatrolPoint.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"


UBTTask_GetRandomPatrolPoint::UBTTask_GetRandomPatrolPoint()
{
	NodeName = "GetRandomPatrolPoint";
}

EBTNodeResult::Type UBTTask_GetRandomPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* TaskMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;
	
	const FVector Home = BB->GetValueAsVector(TEXT("HomeLocation"));

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	FNavLocation NavLoc;
	if (NavSys && NavSys->GetRandomReachablePointInRadius(Home, PatrolRadius, NavLoc))
	{
		BB->SetValueAsVector(TEXT("TargetLocation"), NavLoc.Location);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
