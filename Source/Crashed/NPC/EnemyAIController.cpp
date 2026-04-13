#include "EnemyAIController.h"
#include "BaseEnemy.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

AEnemyAIController::AEnemyAIController()
{
	BBC = CreateDefaultSubobject<UBlackboardComponent>("Blackboard");
	BTC = CreateDefaultSubobject<UBehaviorTreeComponent>("BehaviorTree");

}

// OnPossess — runs the Behavior Tree and initialises the Blackboard
void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (ABaseEnemy* const Enemy = Cast<ABaseEnemy>(InPawn))
	{
		if (UBehaviorTree* const tree = Enemy->GetBehaviorTree())
		{
			
			BBC->InitializeBlackboard(*tree->BlackboardAsset);
			BTC->StartTree(*tree);
			
		}
	}
}


