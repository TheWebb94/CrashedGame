// Fill out your copyright notice in the Description page of Project Settings.

#include "AIController.h"
#include "Components/SplineComponent.h"
#include "NPC\BaseEnemy.h"
#include "NPC\PatrolPath.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"


#include "BTTask_GetPathPoints.h"

UBTTask_GetPathPoints::UBTTask_GetPathPoints()
{
	bCreateNodeInstance = true;
	NodeName = "GetPathPoints";

}

EBTNodeResult::Type UBTTask_GetPathPoints::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// check if AIController or Blackboard exist in the AI Character
	const UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();
	AAIController* MyController = OwnerComp.GetAIOwner();
	if (!MyController || !MyBlackboard)
		return EBTNodeResult::Failed;
	//check if the pawn of the AIController is an instance of AI character
	ABaseEnemy* Enemy = Cast<ABaseEnemy>(MyController->GetPawn());
	if (!Enemy)
		return EBTNodeResult::Failed;
	//Check if PatrolPath reference is valid and locations are populated
	APatrolPath* PathRef = Cast<APatrolPath>(Enemy->PatrolPath);
	if (!PathRef || PathRef->Locations.Num() < 1)
		return EBTNodeResult::Succeeded;
	//Set the MoveToLocation Blackboard key to be the spline point
	OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Vector> ("TargetLocation", PathRef->Locations[Index]);
	if (Index < PathRef->Locations.Num() - 1) {
		Index++;
		return EBTNodeResult::Succeeded;
	}
	Index = 1;
	return EBTNodeResult::Succeeded;

}
