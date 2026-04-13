#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

UCLASS()
class CRASHED_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	static const FName BB_TargetActor;  
	static const FName BB_TargetLocation;
	static const FName BB_IsReturningHome;   
	static const FName BB_HiveLocation;     
	static const FName BB_TargetAnt;         

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;


	virtual void UpdateBlackboard();
};