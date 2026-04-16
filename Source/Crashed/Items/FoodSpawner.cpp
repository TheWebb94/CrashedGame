#include "FoodSpawner.h"
#include "Food.h"
#include "Components/BillboardComponent.h"
#include "Engine/World.h"

AFoodSpawner::AFoodSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	Marker = CreateDefaultSubobject<UBillboardComponent>(TEXT("Marker"));
	RootComponent = Marker;
}

void AFoodSpawner::BeginPlay()
{
	Super::BeginPlay();

	int spawnDeficit = 2;
	
	for (int32 i = 0; i < MaxFoodCount - spawnDeficit; ++i) // dont spawn all food immediately, alllowing for spawn space at runtime
		SpawnFood();

	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AFoodSpawner::RefreshFood, SpawnInterval, true);
}

void AFoodSpawner::SpawnFood()
{
	if (!FoodClass) return;

	const float Angle  = FMath::FRandRange(0.f, 2.f * PI);
	const float Radius = FMath::FRandRange(0.f, SpawnRadius);
	const FVector Offset(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.f);
	const FVector SpawnLocation = GetActorLocation() + Offset;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AFood* NewFood = GetWorld()->SpawnActor<AFood>(FoodClass, SpawnLocation, FRotator::ZeroRotator, Params);
	if (NewFood)
		ActiveFood.Add(NewFood);
}

void AFoodSpawner::RefreshFood()
{
	ActiveFood.RemoveAll([](const TWeakObjectPtr<AFood>& F)
	{
		return !F.IsValid() || !F->IsAvailable();
	});

	const int32 Deficit = MaxFoodCount - ActiveFood.Num();
	for (int32 i = 0; i < Deficit; ++i)
		SpawnFood();
}
