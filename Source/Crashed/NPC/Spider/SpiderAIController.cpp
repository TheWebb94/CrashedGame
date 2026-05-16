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
#include "Crashed/NPC/Ants/AntQueen.h"
#include "Crashed/NPC/Bee/Bee.h"
#include "GameFramework/CharacterMovementComponent.h"

ASpiderAIController::ASpiderAIController()
{
    PrimaryActorTick.bCanEverTick = false;

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

// Risk score (0-1)
// Player present in radius    = +0.50
// Each soldier ant in radius  = +0.30
// Each worker/healer ant      = +0.10
// Each bee in radius          = +0.25
float ASpiderAIController::CalculateRiskScore() const
{
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return 0.f;

    const FVector Origin = MyPawn->GetActorLocation();
    float Risk = 0.f;

    APlayerCharacter* Player = Cast<APlayerCharacter>(
        UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (Player && FVector::Dist(Origin, Player->GetActorLocation()) <= ThreatScanRadius)
        Risk += 0.5f;

    TArray<AActor*> FoundAnts;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AForestAnt::StaticClass(), FoundAnts);
    for (AActor* Actor : FoundAnts)
    {
        AForestAnt* Ant = Cast<AForestAnt>(Actor);
        if (!Ant || FVector::Dist(Origin, Ant->GetActorLocation()) > ThreatScanRadius) continue;
        Risk += (Ant->AntType == EAntType::Soldier) ? 0.3f : 0.1f;
    }

    TArray<AActor*> FoundQueens;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAntQueen::StaticClass(), FoundQueens);
    for (AActor* A : FoundQueens)
    {
        if (FVector::Dist(Origin, A->GetActorLocation()) <= ThreatScanRadius)
            Risk += 0.3f;
    }

    TArray<AActor*> FoundBees;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABee::StaticClass(), FoundBees);
    for (AActor* A : FoundBees)
    {
        if (FVector::Dist(Origin, A->GetActorLocation()) <= ThreatScanRadius)
            Risk += 0.25f;
    }

    return FMath::Clamp(Risk, 0.f, 1.f);
}

// Priority: entangled > Player > AntQueen > Bee > nearest ForestAnt
AActor* ASpiderAIController::FindBestTarget() const
{
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return nullptr;

    const FVector Origin = MyPawn->GetActorLocation();

    TArray<ACharacter*> Candidates;

    APlayerCharacter* Player = Cast<APlayerCharacter>(
        UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (Player && FVector::Dist(Origin, Player->GetActorLocation()) <= ThreatScanRadius)
        Candidates.Add(Player);

    TArray<AActor*> FoundAnts;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AForestAnt::StaticClass(), FoundAnts);
    for (AActor* A : FoundAnts)
    {
        ACharacter* Char = Cast<ACharacter>(A);
        if (Char && FVector::Dist(Origin, Char->GetActorLocation()) <= ThreatScanRadius)
            Candidates.Add(Char);
    }

    TArray<AActor*> FoundQueens;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAntQueen::StaticClass(), FoundQueens);
    for (AActor* A : FoundQueens)
    {
        ACharacter* Char = Cast<ACharacter>(A);
        if (Char && FVector::Dist(Origin, Char->GetActorLocation()) <= ThreatScanRadius)
            Candidates.Add(Char);
    }

    TArray<AActor*> FoundBees;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABee::StaticClass(), FoundBees);
    for (AActor* A : FoundBees)
    {
        ACharacter* Char = Cast<ACharacter>(A);
        if (Char && FVector::Dist(Origin, Char->GetActorLocation()) <= ThreatScanRadius)
            Candidates.Add(Char);
    }

    // Priority 1: nearest entangled target
    ACharacter* NearestEntangled = nullptr;
    float NearestEntangledDist = FLT_MAX;
    for (ACharacter* C : Candidates)
    {
        UCharacterMovementComponent* Move = C->GetCharacterMovement();
        if (Move && Move->MaxWalkSpeed <= 0.f)
        {
            float D = FVector::Dist(Origin, C->GetActorLocation());
            if (D < NearestEntangledDist) { NearestEntangledDist = D; NearestEntangled = C; }
        }
    }
    if (NearestEntangled) return NearestEntangled;

    // Priority 2: player
    if (Player && FVector::Dist(Origin, Player->GetActorLocation()) <= ThreatScanRadius)
        return Player;

    // Priority 3: AntQueen
    AActor* NearestQueen = nullptr;
    float NearestQueenDist = ThreatScanRadius;
    for (AActor* A : FoundQueens)
    {
        float D = FVector::Dist(Origin, A->GetActorLocation());
        if (D < NearestQueenDist) { NearestQueenDist = D; NearestQueen = A; }
    }
    if (NearestQueen) return NearestQueen;

    // Priority 4: nearest Bee
    AActor* NearestBee = nullptr;
    float NearestBeeDist = ThreatScanRadius;
    for (AActor* A : FoundBees)
    {
        float D = FVector::Dist(Origin, A->GetActorLocation());
        if (D < NearestBeeDist) { NearestBeeDist = D; NearestBee = A; }
    }
    if (NearestBee) return NearestBee;

    // Priority 5: nearest ForestAnt
    AActor* Nearest = nullptr;
    float NearestDist = ThreatScanRadius;
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

    FNavLocation NavLoc;
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (NavSys && NavSys->ProjectPointToNavigation(Candidate, NavLoc))
        return NavLoc.Location;

    return Candidate;
}

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

    bool bTargetEntangled = false;
    if (Target)
    {
        ACharacter* TargetChar = Cast<ACharacter>(Target);
        if (TargetChar && TargetChar->GetCharacterMovement())
            bTargetEntangled = TargetChar->GetCharacterMovement()->MaxWalkSpeed <= 0.f;
    }

    float PatrolScore = (1.f - Risk) * 0.5f + HealthNorm * 0.3f;
    if (Target) PatrolScore *= 0.1f;

    float RangedScore = 0.f;
    if (Target && Spider->bCanRangedAttack
        && DistToTarget >= MinRangedDistance
        && DistToTarget <= MaxRangedDistance
        && !bTargetEntangled)
    {
        float DistFactor = 1.f - FMath::Abs(DistToTarget - OptimalRangedDistance) / MaxRangedDistance;
        RangedScore = FMath::Max(0.f, DistFactor) * (1.f - Risk * 0.3f);
    }

    const float EffectiveMeleeRange = Spider->AttackRange;
    float MeleeScore = 0.f;
    if (Target)
    {
        float ApproachFactor = 1.f - FMath::Clamp(DistToTarget / ThreatScanRadius, 0.f, 1.f);
        float MeleeFactor = DistToTarget < EffectiveMeleeRange
            ? 1.f - DistToTarget / EffectiveMeleeRange
            : 0.f;
        float BaseScore = FMath::Max(ApproachFactor, MeleeFactor) * HealthNorm;
        MeleeScore = bTargetEntangled ? FMath::Min(BaseScore * 2.5f, 1.f) : BaseScore;
    }

    float RetreatScore = Risk * 0.6f + (1.f - HealthNorm) * 0.4f;

    float WPatrol  = PatrolScore  * PatrolWeight;
    float WRanged  = RangedScore  * RangedWeight;
    float WMelee   = MeleeScore   * MeleeWeight;
    float WRetreat = RetreatScore * RetreatWeight;

    ESpiderState NewState = ESpiderState::Patrol;
    float Best = WPatrol;
    if (WRanged  > Best) { Best = WRanged;  NewState = ESpiderState::RangedAttack; }
    if (WMelee   > Best) { Best = WMelee;   NewState = ESpiderState::MeleeAttack;  }
    if (WRetreat > Best) { Best = WRetreat; NewState = ESpiderState::Retreat;      }

    if (NewState != ESpiderState::Retreat)
        bWebPlacedThisRetreat = false;

    CurrentState = NewState;

    switch (CurrentState)
    {
    case ESpiderState::Patrol:
        if (Spider->NextWayPoint)
            MoveToActor(Spider->NextWayPoint, 5.f);
        break;

    case ESpiderState::RangedAttack:
        if (Target)
        {
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