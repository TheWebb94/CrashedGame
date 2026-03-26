#include "EnemyAIController.h"
#include "BaseEnemy.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

// Blackboard key name constants — reference these from BT Tasks
const FName AEnemyAIController::BB_PlayerActor    = TEXT("PlayerActor");
const FName AEnemyAIController::BB_TargetLocation = TEXT("TargetLocation");

// OnPossess — runs the Behavior Tree and initialises the Blackboard
void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (ABaseEnemy* const Enemy = Cast<ABaseEnemy>(InPawn))
	{
		if (UBehaviorTree* const tree = Enemy->GetBehaviorTree())
		{
			UBlackboardComponent* b;
			UseBlackboard(tree->BlackboardAsset, b);
			Blackboard = b;
			RunBehaviorTree(tree);
		}
	}
	
	/*
	ABaseEnemy* Enemy = Cast<ABaseEnemy>(InPawn);
	if (Enemy && Enemy->BehaviorTreeAsset)
	{
		DetectionRadius = Enemy->DetectionRadius;
		RunBehaviorTree(Enemy->BehaviorTreeAsset);
	}*/
}

// Tick — keeps the Blackboard up to date for the BT to read
void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateBlackboard();
}

void AEnemyAIController::UpdateBlackboard()
{
	if (!GetBlackboardComponent()) return;

	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!IsValid(Player))
	{
		GetBlackboardComponent()->ClearValue(BB_PlayerActor);
		GetBlackboardComponent()->ClearValue(BB_TargetLocation);
		return;
	}

	const APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	const float DistSq = FVector::DistSquared(ControlledPawn->GetActorLocation(), Player->GetActorLocation());
	if (DistSq <= DetectionRadius * DetectionRadius)
	{
		GetBlackboardComponent()->SetValueAsObject(BB_PlayerActor, Player);
		GetBlackboardComponent()->SetValueAsVector(BB_TargetLocation, Player->GetActorLocation());
	}
	else
	{
		GetBlackboardComponent()->ClearValue(BB_PlayerActor);
		GetBlackboardComponent()->ClearValue(BB_TargetLocation);
	}
}
