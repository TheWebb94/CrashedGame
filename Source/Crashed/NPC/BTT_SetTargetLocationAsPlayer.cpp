// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_SetTargetLocationAsPlayer.h"

UBTT_SetTargetLocationAsPlayer::UBTT_SetTargetLocationAsPlayer()
{
	NodeName = "SetTargetLocationAsPlayer";	
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTT_SetTargetLocationAsPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	
	AAIController* controller = OwnerComp.GetAIOwner();
	
	if (!controller)
	{
		return EBTNodeResult::Failed;
	}
	
	
	
	
	return Super::ExecuteTask(OwnerComp, NodeMemory);
}
