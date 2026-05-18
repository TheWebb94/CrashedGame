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
	//config
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	
	//cast guards
	if (!BB) return EBTNodeResult::Failed;
	
	FVector Home = BB->GetValueAsVector(TEXT("HomeLocation")); //set homeloc
	//set values
		if (Home.IsNearlyZero())
		if (AAIController* C = OwnerComp.GetAIOwner())
			if (APawn* P = C->GetPawn())
				Home = P->GetActorLocation();

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	FNavLocation NavLoc;
	
	//find a random point within patrol radius to set as the target location
	if (NavSys && NavSys->GetRandomReachablePointInRadius(Home, PatrolRadius, NavLoc))
	{
		BB->SetValueAsVector(TEXT("TargetLocation"), NavLoc.Location);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
