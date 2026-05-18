#include "BTService_FindNearbyFood.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Crashed/Items/Food.h"
#include "Kismet/GameplayStatics.h"         

UBTService_FindNearbyFood::UBTService_FindNearbyFood()
{
	NodeName = "FindNearbyFood";
	Interval = 0.5f;
	RandomDeviation = 0.1f;
}

void UBTService_FindNearbyFood::TickNode(UBehaviorTreeComponent& OwnerComp,
										  uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	//config
	AAIController* Controller = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	
	//cast guards
	if (!Controller || !BB)
		return;

	APawn* Pawn = Controller->GetPawn();
	if (!Pawn)
		return;

	//create a list of all food items spawned in game
	TArray<AActor*> AllFood;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFood::StaticClass(), AllFood);

	
	AFood* Closest = nullptr;
	float ClosestDistSq = ScanRadius * ScanRadius;   
	const FVector PawnLoc = Pawn->GetActorLocation();

	//loop through to find the nearest valid food
	for (AActor* Actor : AllFood)
	{
		AFood* Food = Cast<AFood>(Actor);
		if (!Food || !Food->IsAvailable()) //cast guard
			continue;

		const float DistSq = FVector::DistSquared(PawnLoc, Food->GetActorLocation());
		if (DistSq < ClosestDistSq) //cast guard
		{
			ClosestDistSq = DistSq;
			Closest = Food; //if this is the closest of evaluated options, set to closest
		}
	}

	if (Closest) //if valid food found, closest is set tot target
	{
		BB->SetValueAsObject(TEXT("FoodTarget"), Closest);
		BB->SetValueAsBool(TEXT("HasFoodTarget"), true);
	}
	else
	{
		BB->SetValueAsObject(TEXT("FoodTarget"), nullptr);
		BB->SetValueAsBool(TEXT("HasFoodTarget"), false);
	}
}
