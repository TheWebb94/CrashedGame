#include "Bee.h"
#include "BeeAIController.h"
#include "BeeHive.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Crashed/HealthComponent.h"

ABee::ABee()
{
	AIControllerClass = ABeeAIController::StaticClass();

	GetCharacterMovement()->bOrientRotationToMovement = true;

	MoveSpeed       = 450.f;
	AttackRange     = 80.f;
	AttackDamage    = 5.f;
	AttackRate      = 2.f;
	DetectionRadius = 800.f;
	
		
	//Set speration values
	SeparationSphere = CreateDefaultSubobject<USphereComponent>(TEXT("SeparationSphere"));
	SeparationSphere->SetupAttachment(RootComponent);
	SeparationSphere->SetSphereRadius(SeparationSphereRadius);
	SeparationSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SeparationSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	SeparationSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SeparationSphere->SetGenerateOverlapEvents(true);
}

void ABee::BeginPlay()
{
	Super::BeginPlay();
	
	PreviousHealth = HealthComponent ? HealthComponent->GetCurrentHealth() : 0.f;
	if (HealthComponent)
		HealthComponent->OnHealthChanged.AddDynamic(this, &ABee::OnBeeHealthChanged);
}

void ABee::PerformAttack_Implementation()
{
	if (!CurrentAttackTarget) return;
	if (FVector::DistSquared(GetActorLocation(), CurrentAttackTarget->GetActorLocation())
			> AttackRange * AttackRange) return;

	if (UHealthComponent* HC = CurrentAttackTarget->FindComponentByClass<UHealthComponent>())
		HC->ApplyDamageFrom(AttackDamage, this);
	
	HealthComponent->ApplyDamage(HealthComponent->currentHealth); //kill self
}

void ABee::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsDefending) return;

	TArray<AActor*> Overlapping;
	SeparationSphere->GetOverlappingActors(Overlapping, ABee::StaticClass());

	FVector InfluenceVector = FVector::ZeroVector;
	int32   Count       = 0;
	for (AActor* Actor : Overlapping)
	{
		if (Actor == this) continue;
		InfluenceVector += Actor->GetActorLocation();
		++Count;
	}
	if (Count == 0) return;

	const FVector Centroid         = InfluenceVector / (float)Count;
	const FVector AwayFromCentroid = GetActorLocation() - Centroid;
	if (AwayFromCentroid.IsNearlyZero()) return;

	const float Distance = AwayFromCentroid.Size();
	const float Weight   = FMath::Clamp(1.f - (Distance / SeparationSphereRadius), 0.f, 1.f);

	GetCharacterMovement()->Velocity += AwayFromCentroid.GetSafeNormal()
		* SeparationStrength * Weight * DeltaTime;
}

void ABee::OnBeeHealthChanged(float NewHealth, float MaxHealth)
{
	if (NewHealth >= PreviousHealth) { PreviousHealth = NewHealth; return; }
	PreviousHealth = NewHealth;

	if (bIsDefending) return;

	AActor* Attacker = HealthComponent ? HealthComponent->LastInstigator : nullptr;
	if (!Attacker) return;

	if (OwnerHive.IsValid())
		OwnerHive->SendBeesToAttack(Attacker);
}
