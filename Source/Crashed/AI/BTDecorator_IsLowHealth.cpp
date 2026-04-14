// Fill out your copyright notice in the Description page of Project Settings.


#include "BTDecorator_IsLowHealth.h"

#include "Crashed/HealthComponent.h"
#include "Crashed/NPC/BaseEnemy.h"
#include "Crashed/NPC/EnemyAIController.h"

UBTDecorator_IsLowHealth::UBTDecorator_IsLowHealth()
{
	NodeName = "IsLowHealth";
	bCreateNodeInstance = false;
}

bool UBTDecorator_IsLowHealth::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	//check if AIController or Blackboard exist in the AI Character
	const UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();
	AAIController* MyController = OwnerComp.GetAIOwner();
	if (!MyController || !MyBlackboard)
		return false;
	
	//check if the pawn of the AIController is an instance of BaseEnemy
	ABaseEnemy* Enemy = Cast<ABaseEnemy>(MyController->GetPawn());
	if (!Enemy)
		return false;
	
	UHealthComponent* HealthComponent = Enemy->GetHealthComponent();
	if (!HealthComponent) return false;
	
	if (HealthComponent->GetHealthPercent() <= 0.8f)
	{
		return true;
	}
	
	
	return Super::CalculateRawConditionValue(OwnerComp, NodeMemory);
}


