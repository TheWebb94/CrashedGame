#include "BeeHive.h"
#include "ForestBee.h"
#include "Crashed/HealthComponent.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

ABeeHive::ABeeHive()
{
	PrimaryActorTick.bCanEverTick = false;

	HiveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HiveMesh"));
	RootComponent = HiveMesh;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void ABeeHive::BeginPlay()
{
	Super::BeginPlay();

	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &ABeeHive::OnHiveDamaged);
	}
}

void ABeeHive::OnHiveDamaged(float NewHealth, float MaxHealth)
{
	if (!bAlerted && NewHealth > 0.f)
	{
		SpawnSwarm();
	}
}

void ABeeHive::SpawnSwarm()
{
	if (!BeeClass) return;

	bAlerted = true;
	GetWorldTimerManager().SetTimer(AlertCooldownTimer, this, &ABeeHive::ResetAlert, AlertCooldown, false);

	for (int32 i = 0; i < BeesPerSwarm; ++i)
	{
		const FVector RandDir = FMath::VRand();
		const FVector SpawnOffset = FVector(RandDir.X, RandDir.Y, 0.f) * FMath::FRandRange(50.f, BeeSpawnRadius);
		const FVector SpawnLocation = GetActorLocation() + SpawnOffset + FVector(0,0,50.f);

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AForestBee* Bee = GetWorld()->SpawnActor<AForestBee>(BeeClass, SpawnLocation, FRotator::ZeroRotator, Params);
		if (Bee)
		{
			Bee->SetOwningHive(this);
			Bee->ActivateSwarmMode();
			AliveBees.Add(Bee);
		}
	}
}

void ABeeHive::ResetAlert()
{
	bAlerted = false;
}

void ABeeHive::OnBeeDied(ABaseEnemy* DeadBee)
{
	AForestBee* DeadForestBee = Cast<AForestBee>(DeadBee);
	AliveBees.RemoveAll([DeadForestBee](const TWeakObjectPtr<AForestBee>& P){ return P.Get() == DeadForestBee; });
}
