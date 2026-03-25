#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

UCLASS()
class CRASHED_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	// Blackboard key names — reference these from BT Tasks to avoid magic strings
	static const FName BB_PlayerActor;    // Object: the player pawn
	static const FName BB_TargetLocation; // Vector: last known player location

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

private:
	void UpdateBlackboard();

	// Cached from the enemy on possess
	float DetectionRadius = 0.f;
};
