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
    AIControllerClass = AEnemyAIController::StaticClass(); //constructor setup
}

void AForestAnt::BeginPlay()
{
    Super::BeginPlay();

    PreviousHealth = HealthComponent ? HealthComponent->GetCurrentHealth() : 0.f;

    // Workers alert nearby soldiers when they take damage, setup of event 
    if (AntType == EAntType::Worker && HealthComponent)
        HealthComponent->OnHealthChanged.AddDynamic(this, &AForestAnt::OnWorkerHealthChanged);
}

//called on creation by queen to track this ant in her array
//uses a pointer to the queen
void AForestAnt::RegisterWithQueen(AAntQueen* InQueen)
{
    Queen = InQueen;
    if (InQueen)
        OnEnemyDeath.AddDynamic(InQueen, &AAntQueen::OnAntDied); //bind an event for when the ant dies so tracking remains correct in queen
}

//pickup food from ffood source, amount is setup on the food itself
void AForestAnt::SetCarryingFood(bool bCarrying, float Amount)
{
    bIsCarryingFood   = bCarrying;
    CarriedFoodAmount = bCarrying ? Amount : 0.f;
}

//called on creation of the ant by queen
//with pointer to the home hive the queen lives in
void AForestAnt::SetHomeHive(AAntHive* InHive)
{
    //config
    HomeHive = InHive;
    //validity check
    if (InHive)
    {
        if (AEnemyAIController* AIC = Cast<AEnemyAIController>(GetController()))
        {
            if (AIC->BBC)
                AIC->BBC->SetValueAsVector(TEXT("HomeLocation"), InHive->GetActorLocation()); //set value on BB
        }
    }
}

//wwhen ant is spawned by queen, get given appropriate patrol type to maintain a ratio on the queen
void AForestAnt::SetSoldierPatrolType(ESoldierPatrolType Type)
{
    //config
    SoldierPatrolType = Type;
    
    //validity check
    if (AEnemyAIController* AIC = Cast<AEnemyAIController>(GetController()))
    {
        if (AIC->BBC)
            AIC->BBC->SetValueAsBool(TEXT("UsePathPatrol"), Type == ESoldierPatrolType::PathPatrol); //set value
    }
}

//called by the queen when there is an aggressor to the hive
void AForestAnt::SetDefendHive(bool bDefend)
{
    if (AEnemyAIController* AIC = Cast<AEnemyAIController>(GetController()))
    {
        if (AIC->BBC)
            AIC->BBC->SetValueAsBool(TEXT("DefendHive"), bDefend);
    }
}

//set by the queen, the aggressor to the hive is passed as the target
void AForestAnt::SetAttackTarget(AActor* Target)
{
    //config
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

//Any time the ant takes damage, alert hive to nearby threat
void AForestAnt::OnWorkerHealthChanged(float NewHealth, float MaxHealth)
{
    if (NewHealth < PreviousHealth)
        AlertNearbyThreat();
    PreviousHealth = NewHealth;
}

//alerts nearby soldiers of what unit hurt it
void AForestAnt::AlertNearbyThreat()
{
    if (!Queen.IsValid()) return;

    AActor* Attacker = (HealthComponent && HealthComponent->LastInstigator) //fetch last aggressor from health component
        ? HealthComponent->LastInstigator
        : UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

    Queen->AlertSoldiersNearby(GetActorLocation(), Attacker, WorkerAlertRadius);
}

void AForestAnt::OnDeath_Implementation()
{
    Super::OnDeath_Implementation(); //override in childs
}

void AForestAnt::PerformAttack_Implementation()
{
    if (AntType == EAntType::Healer) return; //healers cant attack

    //config
    AActor* Target = CurrentAttackTarget;
    //validity check
    if (!Target) return;

    //check if target is within attack range
    FVector Delta = Target->GetActorLocation() - GetActorLocation();
    Delta.Z = 0.f;
    if (Delta.SizeSquared() > AttackRange * AttackRange) return;

    //apply damage to target
    if (UHealthComponent* HC = Target->FindComponentByClass<UHealthComponent>())
        HC->ApplyDamageFrom(AttackDamage, this);
}

void AForestAnt::Tick(float DeltaTime)
{
    //config
    Super::Tick(DeltaTime);
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 480.f, 0.f);
    bUseControllerRotationYaw = false;
}
