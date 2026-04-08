#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

UCLASS()
class CRASHED_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	static const FName BB_PlayerActor;
	static const FName BB_TargetLocation;

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

	// Protected + virtual so subclasses (e.g. AAntAIController) can override it
	virtual void UpdateBlackboard();
};