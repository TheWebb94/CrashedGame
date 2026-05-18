#include "BTTask_CalcFleeDestination.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"

UBTTask_CalcFleeDestination::UBTTask_CalcFleeDestination()
{
	NodeName = "CalcFleeDestination";
}

EBTNodeResult::Type UBTTask_CalcFleeDestination::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
															   uint8* NodeMemory)
{
	//config
	AAIController* Controller = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB  = OwnerComp.GetBlackboardComponent();
	
	//cast guards
	if (!Controller || !BB) return EBTNodeResult::Failed;

	APawn* Pawn = Controller->GetPawn();
	if (!Pawn)  return EBTNodeResult::Failed;

	const FVector AntLoc    = Pawn->GetActorLocation();
	const FVector Threat    = BB->GetValueAsVector(TEXT("ThreatLocation"));
	const FVector FleeDir   = (AntLoc - Threat).GetSafeNormal2D();
	const FVector FleePoint = AntLoc + FleeDir * FleeDistance;

	// Project onto navmesh so MoveTo doesn't fail on geometry
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	FNavLocation NavLoc;
	
	//set flee point 
	if (NavSys && NavSys->ProjectPointToNavigation(FleePoint, NavLoc, FVector(50.f, 50.f, 100.f)))
		BB->SetValueAsVector(TEXT("FleeDestination"), NavLoc.Location);
	else
		BB->SetValueAsVector(TEXT("FleeDestination"), FleePoint);

	return EBTNodeResult::Succeeded;
}
