#include "SpiderAIController.h"

#include "Spider.h"
#include "Waypoint.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Crashed/NPC/Ants/ForestAnt.h"
#include "Crashed/Player/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Crashed/HealthComponent.h"

ASpiderAIController::ASpiderAIController()
{
    PrimaryActorTick.bCanEverTick = false; // we use a timer instead of Tick

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception")));

    SightConfig->SightRadius                            = AISightRadius;
    SightConfig->LoseSightRadius                        = AILoseSightRadius;
    SightConfig->PeripheralVisionAngleDegrees           = AIFieldOfView;
    SightConfig->SetMaxAge(AISightAge);
    SightConfig->DetectionByAffiliation.bDetectEnemies    = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals   = true;

    GetPerceptionComponent()->SetDominantSense(*SightConfig->GetSenseImplementation());
    GetPerceptionComponent()->ConfigureSense(*SightConfig);
}

void ASpiderAIController::BeginPlay()
{
    Super::BeginPlay();
}

void ASpiderAIController::OnPossess(APawn* MyPawn)
{
    Super::OnPossess(MyPawn);

    GetWorldTimerManager().SetTimer(DecisionTimerHandle, this,
        &ASpiderAIController::EvaluateAndExecute, DecisionInterval, true);
}

FRotator ASpiderAIController::GetControlRotation() const
{
    if (!GetPawn()) return FRotator::ZeroRotator;
    return FRotator(0.f, GetPawn()->GetActorRotation().Yaw, 0.f);
}

//calculates the 'risk score' for the area
// Risk score  (0-1)
// Player present in radius = +0.50
// Each soldier ant in radius = +0.30  (capped so ants alone top at ~0.50)
// Each worker/healer ant   = +0.10
float ASpiderAIController::CalculateRiskScore() const
{
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return 0.f;

    const FVector Origin = MyPawn->GetActorLocation();
    float Risk = 0.f;

    // Check player
    APlayerCharacter* Player = Cast<APlayerCharacter>(
        UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (Player && FVector::Dist(Origin, Player->GetActorLocation()) <= ThreatScanRadius)
        Risk += 0.5f;

    // Check ants
    TArray<AActor*> FoundAnts;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AForestAnt::StaticClass(), FoundAnts);
    for (AActor* Actor : FoundAnts)
    {
        AForestAnt* Ant = Cast<AForestAnt>(Actor);
        if (!Ant || FVector::Dist(Origin, Ant->GetActorLocation()) > ThreatScanRadius) continue;

        Risk += (Ant->AntType == EAntType::Soldier) ? 0.3f : 0.1f;
    }

    return FMath::Clamp(Risk, 0.f, 1.f);
}


// Best attack target: player first, then nearest ForestAnt
AActor* ASpiderAIController::FindBestTarget() const
{
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return nullptr;

    const FVector Origin = MyPawn->GetActorLocation();

    APlayerCharacter* Player = Cast<APlayerCharacter>(
        UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (Player && FVector::Dist(Origin, Player->GetActorLocation()) <= ThreatScanRadius)
        return Player;

    // Nearest ant
    TArray<AActor*> FoundAnts;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AForestAnt::StaticClass(), FoundAnts);
    AActor* Nearest = nullptr;
    float   NearestDist = ThreatScanRadius;
    for (AActor* A : FoundAnts)
    {
        float D = FVector::Dist(Origin, A->GetActorLocation());
        if (D < NearestDist) { NearestDist = D; Nearest = A; }
    }
    return Nearest;
}

FVector ASpiderAIController::CalcFleeDestination(const FVector& ThreatOrigin) const
{
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return MyPawn->GetActorLocation();

    FVector FleeDir = (MyPawn->GetActorLocation() - ThreatOrigin).GetSafeNormal();
    FVector Candidate = MyPawn->GetActorLocation() + FleeDir * 800.f;

    // Project onto nav mesh
    FNavLocation NavLoc;
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (NavSys && NavSys->ProjectPointToNavigation(Candidate, NavLoc))
        return NavLoc.Location;

    return Candidate;
}

// ---------------------------------------------------------------------------
// Core: evaluate utility scores, pick winner, execute
// ---------------------------------------------------------------------------
void ASpiderAIController::EvaluateAndExecute()
{
    ASpider* Spider = Cast<ASpider>(GetPawn());
    if (!Spider) return;

    const float HealthNorm = Spider->HealthComponent
        ? Spider->HealthComponent->GetHealthPercent() : 1.f;
    const float Risk       = CalculateRiskScore();
    AActor*     Target     = FindBestTarget();
    const float DistToTarget = Target
        ? FVector::Dist(Spider->GetActorLocation(), Target->GetActorLocation())
        : ThreatScanRadius;

    // --- Utility scores (0-1 range, normalised) ---

    // Patrol: easy/safe situations
    float PatrolScore = (1.f - Risk) * 0.5f + HealthNorm * 0.3f;

    // Ranged: bell-curve peaking at OptimalRangedDistance
    float RangedScore = 0.f;
    if (Target && Spider->bCanRangedAttack
        && DistToTarget >= MinRangedDistance
        && DistToTarget <= MaxRangedDistance)
    {
        float DistFactor = 1.f - FMath::Abs(DistToTarget - OptimalRangedDistance) / MaxRangedDistance;
        RangedScore = FMath::Max(0.f, DistFactor) * (1.f - Risk * 0.3f);
    }

    // Melee: strong up close, drops to 0 at MeleeRange
    float MeleeScore = 0.f;
    if (Target && DistToTarget < MeleeRange)
        MeleeScore = FMath::Max(0.f, 1.f - DistToTarget / MeleeRange) * HealthNorm;

    // Retreat: high risk or low health
    float RetreatScore = Risk * 0.6f + (1.f - HealthNorm) * 0.4f;

    // --- Apply weights (coefficients from slides) ---
    float WPatrol  = PatrolScore  * PatrolWeight;
    float WRanged  = RangedScore  * RangedWeight;
    float WMelee   = MeleeScore   * MeleeWeight;
    float WRetreat = RetreatScore * RetreatWeight;

    // --- Pick winner ---
    ESpiderState NewState = ESpiderState::Patrol;
    float Best = WPatrol;
    if (WRanged  > Best) { Best = WRanged;  NewState = ESpiderState::RangedAttack; }
    if (WMelee   > Best) { Best = WMelee;   NewState = ESpiderState::MeleeAttack;  }
    if (WRetreat > Best) { Best = WRetreat; NewState = ESpiderState::Retreat;       }

    // Reset retreat-web flag when leaving retreat state
    if (NewState != ESpiderState::Retreat)
        bWebPlacedThisRetreat = false;

    CurrentState = NewState;

    // --- Execute ---
    switch (CurrentState)
    {
    case ESpiderState::Patrol:
        if (Spider->NextWayPoint)
            MoveToActor(Spider->NextWayPoint, 5.f);
        break;

    case ESpiderState::RangedAttack:
        if (Target)
        {
            // Position at optimal range
            if (DistToTarget > OptimalRangedDistance + 100.f)
                MoveToActor(Target, OptimalRangedDistance);
            else
                StopMovement();

            Spider->ShootWebProjectile(Target);
        }
        break;

    case ESpiderState::MeleeAttack:
        if (Target)
        {
            MoveToActor(Target, 100.f);
            Spider->CurrentAttackTarget = Target;
            if (DistToTarget <= Spider->AttackRange)
                Spider->PerformAttack();
        }
        break;

    case ESpiderState::Retreat:
    {
        FVector ThreatLoc = Target
            ? Target->GetActorLocation()
            : Spider->GetActorLocation();
        MoveToLocation(CalcFleeDestination(ThreatLoc), 50.f);

        if (!bWebPlacedThisRetreat)
        {
            Spider->PlaceDefensiveWeb();
            bWebPlacedThisRetreat = true;
        }
        break;
    }
    }
}