#include "Bee.h"
#include "BeeAIController.h"
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
}

void ABee::BeginPlay()
{
	Super::BeginPlay();
}

void ABee::PerformAttack_Implementation()
{
	if (!CurrentAttackTarget) return;
	if (FVector::DistSquared(GetActorLocation(), CurrentAttackTarget->GetActorLocation())
			> AttackRange * AttackRange) return;

	if (UHealthComponent* HC = CurrentAttackTarget->FindComponentByClass<UHealthComponent>())
		HC->ApplyDamage(AttackDamage);
}