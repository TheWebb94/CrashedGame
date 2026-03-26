// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "MyPlayerController.generated.h"
#include "EnemyAIController.h"

#include "BTT_SetTargetLocationAsPlayer.generated.h"


/**
 * 
 */
UCLASS()
class CRASHED_API UBTT_SetTargetLocationAsPlayer : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTT_SetTargetLocationAsPlayer();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	
};
