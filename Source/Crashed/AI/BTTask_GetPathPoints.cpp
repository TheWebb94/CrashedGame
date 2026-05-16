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
	AAIController* MyController = OwnerComp.GetAIOwner();
	if (!MyController || !OwnerComp.GetBlackboardComponent())
		return EBTNodeResult::Failed;

	ABaseEnemy* Enemy = Cast<ABaseEnemy>(MyController->GetPawn());
	if (!Enemy)
		return EBTNodeResult::Failed;

	APatrolPath* PathRef = Cast<APatrolPath>(Enemy->PatrolPath);
	if (!PathRef || PathRef->Locations.Num() < 1)
		return EBTNodeResult::Succeeded;

	if (!bIsInitialized)
	{
		Index        = FMath::RandRange(0, PathRef->Locations.Num() - 1);
		bIsInitialized = true;
	}

	OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Vector>("TargetLocation", PathRef->Locations[Index]);

	if (Index < PathRef->Locations.Num() - 1)
		Index++;
	else
		Index = 1;

	return EBTNodeResult::Succeeded;
}
