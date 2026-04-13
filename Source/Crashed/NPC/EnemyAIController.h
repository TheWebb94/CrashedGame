#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

UCLASS()
class CRASHED_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	
	AEnemyAIController();
	virtual void OnPossess(APawn* InPawn) override;
	
	UPROPERTY(transient)
	class UBehaviorTreeComponent* BTC;
	UPROPERTY(transient)
	UBlackboardComponent* BBC;

};