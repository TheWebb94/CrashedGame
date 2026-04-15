#include "BTService_FindNearbyFood.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Crashed/Items/Food.h"
#include "Kismet/KismetSystemLibrary.h"

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

	AAIController* Controller = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!Controller || !BB)
		return;

	APawn* Pawn = Controller->GetPawn();
	if (!Pawn)
		return;

	TArray<AActor*> Overlaps;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		Pawn->GetActorLocation(),
		ScanRadius,
		ObjectTypes,
		AFood::StaticClass(),
		TArray<AActor*>(),
		Overlaps);


	AFood* Closest = nullptr;
	float ClosestDist = FLT_MAX;
	for (AActor* Actor : Overlaps)
	{
		AFood* Food = Cast<AFood>(Actor);
		if (!Food || !Food->IsAvailable())
			continue;

		const float Dist = FVector::DistSquared(Pawn->GetActorLocation(),
												 Food->GetActorLocation());
		if (Dist < ClosestDist)
		{
			ClosestDist = Dist;
			Closest = Food;
		}
	}

	if (Closest)
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
