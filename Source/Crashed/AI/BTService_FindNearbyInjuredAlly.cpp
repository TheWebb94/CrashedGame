#include "BTService_FindNearbyInjuredAlly.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Crashed/NPC/Ants/ForestAnt.h"
#include "Crashed/HealthComponent.h"

UBTService_FindNearbyInjuredAlly::UBTService_FindNearbyInjuredAlly()
{
	NodeName = TEXT("FindNearbyInjuredAlly");
	Interval = 0.5f;
	RandomDeviation = 0.1f;
}

void UBTService_FindNearbyInjuredAlly::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* Controller = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB  = OwnerComp.GetBlackboardComponent();
	if (!Controller || !BB) return;

	APawn* Self = Controller->GetPawn();
	if (!Self) return;

	TArray<AActor*> AllAnts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AForestAnt::StaticClass(), AllAnts);

	AForestAnt* BestTarget    = nullptr;
	float        LowestHealth = HealthThreshold;

	for (AActor* Actor : AllAnts)
	{
		AForestAnt* Ant = Cast<AForestAnt>(Actor);
		if (!Ant || Ant == Self) continue;
		if (FVector::Dist(Self->GetActorLocation(), Ant->GetActorLocation()) > ScanRadius) continue;

		UHealthComponent* HC = Ant->HealthComponent;
		if (!HC) continue;

		const float Pct = HC->GetHealthPercent();
		if (Pct < LowestHealth)
		{
			LowestHealth = Pct;
			BestTarget   = Ant;
		}
	}

	if (BestTarget)
	{
		BB->SetValueAsObject(TEXT("HealTarget"), BestTarget);
		BB->SetValueAsBool(TEXT("HasHealTarget"), true);
	}
	else
	{
		BB->ClearValue(TEXT("HealTarget"));
		BB->SetValueAsBool(TEXT("HasHealTarget"), false);
	}
}
