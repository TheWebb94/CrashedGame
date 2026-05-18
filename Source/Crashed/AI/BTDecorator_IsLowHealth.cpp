#include "BTDecorator_IsLowHealth.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Crashed/HealthComponent.h"
#include "Crashed/NPC/BaseEnemy.h"
#include "Crashed/NPC/EnemyAIController.h"

UBTDecorator_IsLowHealth::UBTDecorator_IsLowHealth()
{
	NodeName = "IsLowHealth";
	bCreateNodeInstance  = true;   // each ant needs its own instance to store the delegate binding to prevent ants reacting to another ants lowhealth status
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant  = true;
}

///<summary>checks the health percent of the unit possessed, and checks against their configured lowHealthThreshold. Different units have differnt low health tolerances before changing behaviours</summary>
bool UBTDecorator_IsLowHealth::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
														   uint8* NodeMemory) const
{
	//cast guards, prevents progressing through the method with invalid data
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

//Onbecomerelevant fires when the node decorated by this decorator updates - this allows us to immediately evaluate health-status impact directly from the healthcomponent
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

//this removes the listening in to the tree, prevents duplicate binidings building up as tree node is called, exited, and recalled.
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
