#pragma once

#include "CoreMinimal.h"
#include "Crashed/NPC/EnemyAIController.h"
#include "AAntAIController.generated.h"

UCLASS()
class CRASHED_API AAntAIController : public AEnemyAIController
{
	GENERATED_BODY()

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void UpdateBlackboard() override;   // already declared
};