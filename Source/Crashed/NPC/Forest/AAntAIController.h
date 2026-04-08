#pragma once

#include "CoreMinimal.h"
#include "Crashed/NPC/EnemyAIController.h"
#include "AAntAIController.generated.h"

UCLASS()
class CRASHED_API AAntAIController : public AEnemyAIController
{
	GENERATED_BODY()

protected:
	// When the ant is returning home, redirect BB_TargetLocation to the hive
	virtual void UpdateBlackboard() override;
};