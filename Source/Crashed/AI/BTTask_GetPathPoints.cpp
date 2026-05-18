// Fill out your copyright notice in the Description page of Project Settings.
#include "BTTask_GetPathPoints.h"
#include "AIController.h"
#include "Components/SplineComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "Crashed/NPC/BaseEnemy.h"
#include "Crashed/NPC/PatrolPath.h"

UBTTask_GetPathPoints::UBTTask_GetPathPoints()
{
	bCreateNodeInstance = true;
	NodeName = "GetPathPoints";

}

EBTNodeResult::Type UBTTask_GetPathPoints::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	//config
	AAIController* MyController = OwnerComp.GetAIOwner();
	
	//cast guards
	if (!MyController || !OwnerComp.GetBlackboardComponent())
		return EBTNodeResult::Failed;

	ABaseEnemy* Enemy = Cast<ABaseEnemy>(MyController->GetPawn());
	if (!Enemy)
		return EBTNodeResult::Failed;

	APatrolPath* PathRef = Cast<APatrolPath>(Enemy->PatrolPath); //access the patrol path 
	if (!PathRef || PathRef->Locations.Num() < 1) //cast guard
		return EBTNodeResult::Succeeded;
	
	if (!bIsInitialized)
	{
		Index        = FMath::RandRange(0, PathRef->Locations.Num() - 1); //set a random point in the patrol path as the starting point
		bIsInitialized = true;
	}
	//set as target location
	OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Vector>("TargetLocation", PathRef->Locations[Index]);

	//after setting target location, increase counter so next point is reached
	if (Index < PathRef->Locations.Num() - 1)
		Index++;
	else //reset to start of path once path complete
		Index = 1;

	return EBTNodeResult::Succeeded;
}
