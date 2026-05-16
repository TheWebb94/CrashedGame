#include "Bee.h"
#include "BeeAIController.h"
#include "BeeHive.h"
#include "Components/CapsuleComponent.h"
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
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(
	   ECC_Pawn, ECR_Overlap);
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
