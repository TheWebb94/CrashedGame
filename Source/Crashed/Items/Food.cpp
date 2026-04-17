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
}

void AFood::BeginPlay()
{
    Super::BeginPlay();

    // Roll a random starting value within the configured range
    FoodAmount        = FMath::RandRange(MinStartAmount, MaxStartAmount);
    InitialFoodAmount = FoodAmount;

    // Scale the actor proportionally to how high it rolled
    const float Scale = FMath::GetMappedRangeValueClamped(
        FVector2D(MinStartAmount, MaxStartAmount),
        FVector2D(MinScale, MaxScale),
        FoodAmount);
    float scaleFactor = 2;
    SetActorScale3D(FVector(Scale * scaleFactor));

    UpdateMesh();
}

float AFood::TakeFood()
{
    if (bDepleted)
        return 0.f;

    float amountTaken = 1.0f;

    if (FoodAmount >= amountTaken)
    {
        FoodAmount -= amountTaken;
        UpdateMesh();

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
        FoodAmount  = 0.0f;
        bDepleted   = true;
        FoodMesh->SetVisibility(false);
        FTimerHandle DespawnHandle;
        GetWorldTimerManager().SetTimer(DespawnHandle, this, &AFood::K2_DestroyActor, DespawnDelay, false);
    }

    return amountTaken;
}

void AFood::DecayFood()
{
    FoodAmount = FoodAmount * decayRate;
    UpdateMesh();

    if (FoodAmount <= 0.f)
    {
        bDepleted = true;
        Destroy(true);
    }
}

void AFood::UpdateMesh()
{
    if (InitialFoodAmount <= 0.f) return;

    const float Pct = FoodAmount / InitialFoodAmount;

    UStaticMesh* Target = nullptr;
    if      (Pct > 0.85) Target = MeshWhole;
    else if (Pct > 0.25) Target = MeshSlightlyEaten;
    else                  Target = MeshMostlyEaten;

    if (Target)
        FoodMesh->SetStaticMesh(Target);
}
