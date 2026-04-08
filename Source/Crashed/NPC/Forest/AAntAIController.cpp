#include "AAntAIController.h"
#include "ForestAnt.h"
#include "AntHive.h"

#include "BehaviorTree/BlackboardComponent.h"

void AAntAIController::UpdateBlackboard()
{
	AForestAnt* Ant = Cast<AForestAnt>(GetPawn());

	if (Ant && Ant->bReturningHome && Ant->HomeHive.IsValid())
	{
		if (GetBlackboardComponent())
		{
			// Override the BT target — send the ant home instead of toward the player
			GetBlackboardComponent()->ClearValue(BB_PlayerActor);
			GetBlackboardComponent()->SetValueAsVector(BB_TargetLocation, Ant->HomeHive->GetActorLocation());
		}
		return; // skip normal player-detection logic
	}

	Super::UpdateBlackboard();
}