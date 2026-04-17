#include "BTTask_Attack.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Crashed/NPC/BaseEnemy.h"
#include "Engine/World.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = TEXT("Attack");
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller) return EBTNodeResult::Failed;

	ABaseEnemy* Enemy = Cast<ABaseEnemy>(Controller->GetPawn());
	if (!Enemy) return EBTNodeResult::Failed;

	if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
		Enemy->CurrentAttackTarget = Cast<AActor>(BB->GetValueAsObject(FName("TargetActor")));

	Enemy->PerformAttack();

	const float Delay = (Enemy->AttackRate > 0.f) ? (1.f / Enemy->AttackRate) : 1.f;
	CachedOwnerComp = &OwnerComp;

	Enemy->GetWorldTimerManager().SetTimer(AttackTimerHandle, [this]()
	{
		if (CachedOwnerComp)
		{
			if (AAIController* C = CachedOwnerComp->GetAIOwner())
				if (ABaseEnemy* E = Cast<ABaseEnemy>(C->GetPawn()))
					E->CurrentAttackTarget = nullptr;

			FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
			CachedOwnerComp = nullptr;
		}
	}, Delay, false);

	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBTTask_Attack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* Controller = OwnerComp.GetAIOwner())
		if (ABaseEnemy* Enemy = Cast<ABaseEnemy>(Controller->GetPawn()))
		{
			Enemy->GetWorldTimerManager().ClearTimer(AttackTimerHandle);
			Enemy->CurrentAttackTarget = nullptr;
		}
	CachedOwnerComp = nullptr;
	return EBTNodeResult::Aborted;
}
