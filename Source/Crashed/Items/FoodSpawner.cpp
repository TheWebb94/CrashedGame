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

	// Spawn initial food — bypass pollination for startup stock
	const int32 spawnDeficit = 2;
	for (int32 i = 0; i < MaxFoodCount - spawnDeficit; ++i)
		SpawnFood();

	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this,
		&AFoodSpawner::RefreshFood, SpawnInterval, true);
}

void AFoodSpawner::Pollinate()
{
	bIsPollinated = true;
}

void AFoodSpawner::SpawnFood()
{
	if (!FoodClass) return;

	const float Angle  = FMath::FRandRange(0.f, 2.f * PI);
	const float Radius = FMath::FRandRange(minSpawnRadius, SpawnRadius);
	const FVector Offset(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AFood* NewFood = GetWorld()->SpawnActor<AFood>(FoodClass,
		GetActorLocation() + Offset, FRotator::ZeroRotator, Params);
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
	if (Deficit <= 0) return;

	if (bRequiresPollination)
	{
		if (!bIsPollinated) return;
		bIsPollinated = false; // one pollination → one replenishment cycle
	}

	for (int32 i = 0; i < Deficit; ++i)
		SpawnFood();
}