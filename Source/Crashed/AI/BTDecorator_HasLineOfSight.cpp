// Fill out your copyright notice in the Description page of Project Settings.
#include "BTTask_GetPathPoints.h"
#include "BTDecorator_HasLineOfSight.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "Crashed/NPC/BaseEnemy.h"


UBTDecorator_HasLineOfSight::UBTDecorator_HasLineOfSight()
{
	NodeName = "hasLineOfSight";
	bCreateNodeInstance = true;

}

bool UBTDecorator_HasLineOfSight::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	//check if AIController or Blackboard exist in the AI Character
	const UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();
	AAIController* MyController = OwnerComp.GetAIOwner();
	if (!MyController || !MyBlackboard)
		return false;
	
	//check if the pawn of the AIController is an instance of BaseEnemy
	ABaseEnemy* MyCharacter = Cast<ABaseEnemy>(MyController->GetPawn());
	if (!MyCharacter)
		return false;
	//Get main character detected, push forward for the Engage sequence
	if (MyBlackboard->GetValueAsBool("HasLineOfSight"))
		return true;
	return false;

}
