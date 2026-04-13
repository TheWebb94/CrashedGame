#include "EnemyAIController.h"
#include "BaseEnemy.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

// Blackboard key name constants
const FName AEnemyAIController::BB_TargetActor    = TEXT("TargetActor");
const FName AEnemyAIController::BB_TargetLocation = TEXT("TargetLocation");
const FName AEnemyAIController::BB_IsReturningHome = TEXT("IsReturningHome");
const FName AEnemyAIController::BB_HiveLocation    = TEXT("HiveLocation");
const FName AEnemyAIController::BB_TargetAnt       = TEXT("TargetAnt");

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
		GetBlackboardComponent()->ClearValue(BB_TargetActor);
		GetBlackboardComponent()->ClearValue(BB_TargetLocation);
		return;
	}

	const APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	// Read DetectionRadius directly from the enemy each tick — supports runtime changes
	float EffectiveDetectionRadius = 0.f;
	if (const ABaseEnemy* Enemy = Cast<ABaseEnemy>(ControlledPawn))
	{
		EffectiveDetectionRadius = Enemy->DetectionRadius;
	}

	const float DistSq = FVector::DistSquared(ControlledPawn->GetActorLocation(), Player->GetActorLocation());
	if (DistSq <= EffectiveDetectionRadius * EffectiveDetectionRadius)
	{
		GetBlackboardComponent()->SetValueAsObject(BB_TargetActor, Player);
		GetBlackboardComponent()->SetValueAsVector(BB_TargetLocation, Player->GetActorLocation());
	}
	else
	{
		GetBlackboardComponent()->ClearValue(BB_TargetActor);
		GetBlackboardComponent()->ClearValue(BB_TargetLocation);
	}
}

