#include "ForestAnt.h"
#include "AntQueen.h"
#include "AntHive.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Crashed/HealthComponent.h"
#include "Crashed/NPC/EnemyAIController.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AForestAnt::AForestAnt()
{
    AIControllerClass = AEnemyAIController::StaticClass();
}

void AForestAnt::BeginPlay()
{
    Super::BeginPlay();

    PreviousHealth = HealthComponent ? HealthComponent->GetCurrentHealth() : 0.f;

    // Workers alert nearby soldiers when they take damage
    if (AntType == EAntType::Worker && HealthComponent)
        HealthComponent->OnHealthChanged.AddDynamic(this, &AForestAnt::OnWorkerHealthChanged);
}

void AForestAnt::RegisterWithQueen(AAntQueen* InQueen)
{
    Queen = InQueen;
    if (InQueen)
        OnEnemyDeath.AddDynamic(InQueen, &AAntQueen::OnAntDied);
}

void AForestAnt::SetCarryingFood(bool bCarrying, float Amount)
{
    bIsCarryingFood   = bCarrying;
    CarriedFoodAmount = bCarrying ? Amount : 0.f;
}

void AForestAnt::SetHomeHive(AAntHive* InHive)
{
    HomeHive = InHive;
    if (InHive)
    {
        if (AEnemyAIController* AIC = Cast<AEnemyAIController>(GetController()))
        {
            if (AIC->BBC)
                AIC->BBC->SetValueAsVector(TEXT("HomeLocation"), InHive->GetActorLocation());
        }
    }
}

void AForestAnt::SetSoldierPatrolType(ESoldierPatrolType Type)
{
    SoldierPatrolType = Type;
    if (AEnemyAIController* AIC = Cast<AEnemyAIController>(GetController()))
    {
        if (AIC->BBC)
            AIC->BBC->SetValueAsBool(TEXT("UsePathPatrol"), Type == ESoldierPatrolType::PathPatrol);
    }
}

void AForestAnt::SetDefendHive(bool bDefend)
{
    if (AEnemyAIController* AIC = Cast<AEnemyAIController>(GetController()))
    {
        if (AIC->BBC)
            AIC->BBC->SetValueAsBool(TEXT("DefendHive"), bDefend);
    }
}

void AForestAnt::SetAttackTarget(AActor* Target)
{
    CurrentAttackTarget = Target;
    if (AEnemyAIController* AIC = Cast<AEnemyAIController>(GetController()))
    {
        if (AIC->BBC)
        {
            AIC->BBC->SetValueAsObject(TEXT("TargetActor"), Target);
            AIC->BBC->SetValueAsBool(TEXT("HasLineOfSight"), true);
        }
    }
}

void AForestAnt::OnWorkerHealthChanged(float NewHealth, float MaxHealth)
{
    if (NewHealth < PreviousHealth)
        AlertNearbyThreat();
    PreviousHealth = NewHealth;
}

void AForestAnt::AlertNearbyThreat()
{
    if (!Queen.IsValid()) return;
    // Default to the player as the most likely attacker since HealthComponent doesn't carry attacker information
    APawn* Attacker = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    Queen->AlertSoldiersNearby(GetActorLocation(), Attacker, WorkerAlertRadius);
}

void AForestAnt::OnDeath_Implementation()
{
    Super::OnDeath_Implementation();
}

void AForestAnt::PerformAttack_Implementation()
{
    if (AntType == EAntType::Healer) return;

    AActor* Target = CurrentAttackTarget;
    if (!Target) return;

    FVector Delta = Target->GetActorLocation() - GetActorLocation();
    Delta.Z = 0.f;
    if (Delta.SizeSquared() > AttackRange * AttackRange) return;

    if (UHealthComponent* HC = Target->FindComponentByClass<UHealthComponent>())
        HC->ApplyDamageFrom(AttackDamage, this);
}

void AForestAnt::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 480.f, 0.f);
    bUseControllerRotationYaw = false;
}
