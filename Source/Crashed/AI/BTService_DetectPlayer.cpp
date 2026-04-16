#include "BTService_DetectPlayer.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

UBTService_DetectPlayer::UBTService_DetectPlayer()
{
	NodeName          = "DetectPlayer";
	Interval          = 0.2f;
	RandomDeviation   = 0.05f;
}

void UBTService_DetectPlayer::TickNode(UBehaviorTreeComponent& OwnerComp,
										uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* Controller = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB  = OwnerComp.GetBlackboardComponent();
	if (!Controller || !BB) return;

	APawn* Pawn   = Controller->GetPawn();
	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!Pawn || !Player)
	{
		BB->SetValueAsBool(TEXT("IsPlayerNear"), false);
		return;
	}

	const float DistSq          = FVector::DistSquared(Pawn->GetActorLocation(), Player->GetActorLocation());
	const bool  bCurrentlyFleeing = BB->GetValueAsBool(TEXT("IsPlayerNear"));

	if (!bCurrentlyFleeing && DistSq <= FleeRadius * FleeRadius)
	{
		// Player just entered flee range — start fleeing
		BB->SetValueAsBool(TEXT("IsPlayerNear"), true);
		BB->SetValueAsVector(TEXT("ThreatLocation"), Player->GetActorLocation());
	}
	else if (bCurrentlyFleeing && DistSq > SafeRadius * SafeRadius)
	{
		// Ant is now far enough — stop fleeing
		BB->SetValueAsBool(TEXT("IsPlayerNear"), false);
	}
	else if (bCurrentlyFleeing)
	{
		// Still fleeing — keep threat location current so flee direction stays accurate
		BB->SetValueAsVector(TEXT("ThreatLocation"), Player->GetActorLocation());
	}
}
