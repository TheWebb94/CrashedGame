#include "Food.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

AFood::AFood()
{
	PrimaryActorTick.bCanEverTick = false;

	FoodMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FoodMesh"));
	RootComponent = FoodMesh;

	PickupRadius = CreateDefaultSubobject<USphereComponent>(TEXT("PickupRadius"));
	PickupRadius->SetupAttachment(RootComponent);
	PickupRadius->SetSphereRadius(80.f);
	PickupRadius->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	FoodAmount = FMath::RandRange(5.f, 30.f);
}

float AFood::TakeFood()
{
	if (bDepleted)
		return 0.f;

	float amountTaken = 1.0f;

	if (FoodAmount >= amountTaken)
	{
		FoodAmount -= amountTaken;
		if (FoodAmount <= 0.f)                         
		{
			bDepleted = true;
			FoodMesh->SetVisibility(false);
			FTimerHandle DespawnHandle;
			GetWorldTimerManager().SetTimer(DespawnHandle, this, &AFood::K2_DestroyActor, DespawnDelay, false);
		}
	}
	else
	{
		amountTaken = FoodAmount;
		FoodAmount = 0.0f;
		bDepleted = true;
		FoodMesh->SetVisibility(false);
		FTimerHandle DespawnHandle;
		GetWorldTimerManager().SetTimer(DespawnHandle, this, &AFood::K2_DestroyActor, DespawnDelay, false);
	}

	return amountTaken;
}

void AFood::DecayFood()
{
	FoodAmount = FoodAmount * decayRate;
	
	if (FoodAmount <= 0.5f)
	{
		bDepleted = true;
		Destroy(true);
	}
}
