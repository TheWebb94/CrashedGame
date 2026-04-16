#include "BTDecorator_IsLowHealth.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Crashed/HealthComponent.h"
#include "Crashed/NPC/BaseEnemy.h"
#include "Crashed/NPC/EnemyAIController.h"

UBTDecorator_IsLowHealth::UBTDecorator_IsLowHealth()
{
	NodeName = "IsLowHealth";
	bCreateNodeInstance  = true;   // each ant needs its own instance to store the delegate binding
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant  = true;
}

bool UBTDecorator_IsLowHealth::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
														   uint8* NodeMemory) const
{
	AAIController* MyController = OwnerComp.GetAIOwner();
	if (!MyController)
		return false;

	ABaseEnemy* Enemy = Cast<ABaseEnemy>(MyController->GetPawn());
	if (!Enemy)
		return false;

	UHealthComponent* HC = Enemy->GetHealthComponent();
	if (!HC)
		return false;

	return HC->GetHealthPercent() <= HealthThreshold;
}

void UBTDecorator_IsLowHealth::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller) return;

	ABaseEnemy* Enemy = Cast<ABaseEnemy>(Controller->GetPawn());
	if (!Enemy) return;

	UHealthComponent* HC = Enemy->GetHealthComponent();
	if (!HC) return;

	CachedOwnerComp  = &OwnerComp;
	CachedHealthComp = HC;
	HC->OnHealthChanged.AddDynamic(this, &UBTDecorator_IsLowHealth::OnHealthChanged);
}

void UBTDecorator_IsLowHealth::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);

	if (CachedHealthComp.IsValid())
		CachedHealthComp->OnHealthChanged.RemoveDynamic(this, &UBTDecorator_IsLowHealth::OnHealthChanged);

	CachedOwnerComp  = nullptr;
	CachedHealthComp = nullptr;
}

void UBTDecorator_IsLowHealth::OnHealthChanged(float NewHealth, float MaxHealth)
{
	if (CachedOwnerComp.IsValid())
		ConditionalFlowAbort(*CachedOwnerComp.Get(), EBTDecoratorAbortRequest::ConditionResultChanged);
}
