#include "Food.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

AFood::AFood()
{
    PrimaryActorTick.bCanEverTick = false;

    //setup mesh on object
    FoodMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FoodMesh"));
    RootComponent = FoodMesh;

    //setup the pickup radius on the object
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

///Method for calling from ants when they want to take some food from this source
float AFood::TakeFood()
{
    if (bDepleted)
        return 0.f;

    //ant always take this amount of food
    float amountTaken = 1.0f;

    
    if (FoodAmount >= amountTaken) //if not all of the food is taken
    {
        FoodAmount -= amountTaken; //take the food from the held value
        UpdateMesh(); //changes the mesh to a more consumed version

        if (FoodAmount <= 0.f) //if all is eeaten after taking, delete the object after a delay
        {
            bDepleted = true;
            FoodMesh->SetVisibility(false);
            FTimerHandle DespawnHandle;
            GetWorldTimerManager().SetTimer(DespawnHandle, this, &AFood::K2_DestroyActor, DespawnDelay, false);
        }
    }
    else //if not enough food to take the maximum carry amount, take all that is left, then despawn
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

//food decays naturally over time - did i even implement this? bp?
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

//updates the mesh to the respective version, having a more decayed apple mesh at percent food value thresholds
void AFood::UpdateMesh()
{
    if (InitialFoodAmount <= 0.f) return;

    const float Pct = FoodAmount / InitialFoodAmount;

    UStaticMesh* Target = nullptr;
    if      (Pct > 0.95) Target = MeshWhole;
    else if (Pct > 0.55) Target = MeshSlightlyEaten;
    else                  Target = MeshMostlyEaten;

    if (Target)
        FoodMesh->SetStaticMesh(Target);
}
