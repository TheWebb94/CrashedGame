#include "SpiderAIController.h"

#include "Spider.h"
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
#include "Navigation/PathFollowingComponent.h"

ASpiderAIController::ASpiderAIController()
{
    PrimaryActorTick.bCanEverTick = false;
    
    //config for perception parameters
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception")));

    SightConfig->SightRadius                               = AISightRadius;
    SightConfig->LoseSightRadius                           = AILoseSightRadius;
    SightConfig->PeripheralVisionAngleDegrees              = AIFieldOfView;
    SightConfig->SetMaxAge(AISightAge);
    SightConfig->DetectionByAffiliation.bDetectEnemies     = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies  = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals    = true;

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

    //set a timer to evaluate states for the utility AI
    GetWorldTimerManager().SetTimer(DecisionTimerHandle, this,
        &ASpiderAIController::EvaluateAndExecute, DecisionInterval, true);
}

FRotator ASpiderAIController::GetControlRotation() const
{
    //finds the forward rotation 
    if (!GetPawn()) return FRotator::ZeroRotator;
    return FRotator(0.f, GetPawn()->GetActorRotation().Yaw, 0.f);
}

float ASpiderAIController::CalculateRiskScore() const
{
    //config and guard 
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return 0.f;

    const FVector Origin = MyPawn->GetActorLocation();
    float Risk = 0.f;

    //summates the players contribution to rsik score
    APlayerCharacter* Player = Cast<APlayerCharacter>(
        UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (Player && FVector::Dist(Origin, Player->GetActorLocation()) <= ThreatScanRadius)
        Risk += 0.5f; //player adds 0.5 risk value

    //summates all nearby ants to risk score
    TArray<AActor*> FoundAnts;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AForestAnt::StaticClass(), FoundAnts);
    for (AActor* Actor : FoundAnts)
    {
        AForestAnt* Ant = Cast<AForestAnt>(Actor);
        if (!Ant || FVector::Dist(Origin, Ant->GetActorLocation()) > ThreatScanRadius) continue;
        Risk += (Ant->AntType == EAntType::Soldier) ? 0.3f : 0.1f; //soldiers are worth 0.3, all other ants 0.1 risk
    }

    //summates the queens contribution to risk score
    TArray<AActor*> FoundQueens;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAntQueen::StaticClass(), FoundQueens);
    for (AActor* A : FoundQueens)
    {
        if (FVector::Dist(Origin, A->GetActorLocation()) <= ThreatScanRadius)
            Risk += 0.3f; //worth 0.3 risk
    }

    //summates the bees contribution to risk score
    TArray<AActor*> FoundBees;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABee::StaticClass(), FoundBees);
    for (AActor* A : FoundBees)
    {
        if (FVector::Dist(Origin, A->GetActorLocation()) <= ThreatScanRadius)
            Risk += 0.1f; //worth 0.1 risk per bee
    }

    return FMath::Clamp(Risk, 0.f, 1.f); //clamps the value to a max value of 1
}

//finds the highest priority target within threat scan radius
AActor* ASpiderAIController::FindBestTarget() const
{
    //config and guard
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return nullptr;

    const FVector Origin = MyPawn->GetActorLocation();

    //stores all possible nearby targets
    TArray<ACharacter*> Candidates;

    //adds player to candidates if within threat radius
    APlayerCharacter* Player = Cast<APlayerCharacter>(
        UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (Player && FVector::Dist(Origin, Player->GetActorLocation()) <= ThreatScanRadius)
        Candidates.Add(Player);

    //adds all nearby ants to candidates
    TArray<AActor*> FoundAnts;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AForestAnt::StaticClass(), FoundAnts);
    for (AActor* A : FoundAnts)
    {
        ACharacter* Char = Cast<ACharacter>(A);
        if (Char && FVector::Dist(Origin, Char->GetActorLocation()) <= ThreatScanRadius)
            Candidates.Add(Char);
    }

    //adds all nearby queens to candidates
    TArray<AActor*> FoundQueens;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAntQueen::StaticClass(), FoundQueens);
    for (AActor* A : FoundQueens)
    {
        ACharacter* Char = Cast<ACharacter>(A);
        if (Char && FVector::Dist(Origin, Char->GetActorLocation()) <= ThreatScanRadius)
            Candidates.Add(Char);
    }

    //adds all nearby bees to candidates
    TArray<AActor*> FoundBees;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABee::StaticClass(), FoundBees);
    for (AActor* A : FoundBees)
    {
        ACharacter* Char = Cast<ACharacter>(A);
        if (Char && FVector::Dist(Origin, Char->GetActorLocation()) <= ThreatScanRadius)
            Candidates.Add(Char);
    }

    //Priority 1: nearest entangled target, entangled targets are highest priority as they cannot escape
    ACharacter* NearestEntangled = nullptr;
    float NearestEntangledDist = FLT_MAX;

    for (ACharacter* C : Candidates)
    {
        UCharacterMovementComponent* Move = C->GetCharacterMovement();

        //movement speed of 0 indicates entangled in web
        if (Move && Move->MaxWalkSpeed <= 0.f)
        {
            float D = FVector::Dist(Origin, C->GetActorLocation());

            if (D < NearestEntangledDist)
            {
                NearestEntangledDist = D;
                NearestEntangled = C;
            }
        }
    }

    //returns closest entangled target if one exists
    if (NearestEntangled) return NearestEntangled;

    //Priority 2: player target
    if (Player && FVector::Dist(Origin, Player->GetActorLocation()) <= ThreatScanRadius)
        return Player;

    //Priority 3: nearest queen target
    AActor* NearestQueen = nullptr;
    float NearestQueenDist = ThreatScanRadius;

    for (AActor* A : FoundQueens)
    {
        float D = FVector::Dist(Origin, A->GetActorLocation());

        //stores nearest queen within threat radius
        if (D < NearestQueenDist)
        {
            NearestQueenDist = D;
            NearestQueen = A;
        }
    }

    //returns nearest queen if one exists
    if (NearestQueen) return NearestQueen;

    //Priority 4: nearest bee target
    AActor* NearestBee = nullptr;
    float NearestBeeDist = ThreatScanRadius;

    for (AActor* A : FoundBees)
    {
        float D = FVector::Dist(Origin, A->GetActorLocation());

        //stores nearest bee within threat radius
        if (D < NearestBeeDist)
        {
            NearestBeeDist = D;
            NearestBee = A;
        }
    }

    //returns nearest bee if one exists
    if (NearestBee) return NearestBee;

    //Priority 5: nearest forest ant target
    AActor* Nearest = nullptr;
    float NearestDist = ThreatScanRadius;

    for (AActor* A : FoundAnts)
    {
        float D = FVector::Dist(Origin, A->GetActorLocation());

        //stores nearest ant within threat radius
        if (D < NearestDist)
        {
            NearestDist = D;
            Nearest = A;
        }
    }

    //returns nearest valid ant target
    return Nearest;
}

//calculates a flee destination away from the current threat
FVector ASpiderAIController::CalcFleeDestination(const FVector& ThreatOrigin) const
{
    //config and guard
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return FVector::ZeroVector;

    //calculates flee direction away from threat and projects a flee point ahead
    FVector FleeDir   = (MyPawn->GetActorLocation() - ThreatOrigin).GetSafeNormal();
    FVector Candidate = MyPawn->GetActorLocation() + FleeDir * 800.f;

    //projects flee destination to nearest valid navmesh point
    FNavLocation NavLoc;
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

    //returns valid navigation location if projection succeeds
    if (NavSys && NavSys->ProjectPointToNavigation(Candidate, NavLoc))
        return NavLoc.Location;

    //fallback to original candidate if navmesh projection fails
    return Candidate;
}

//evaluates all utility scores and executes the highest scoring behaviour
void ASpiderAIController::EvaluateAndExecute()
{
    //config and guard
    ASpider* Spider = Cast<ASpider>(GetPawn());
    if (!Spider) return;

    //normalised health value used by utility scoring
    const float HealthNorm = Spider->HealthComponent
        ? Spider->HealthComponent->GetHealthPercent() : 1.f;

    //calculates overall nearby environmental threat level
    const float Risk = CalculateRiskScore();

    //finds the highest priority nearby target
    AActor* Target = FindBestTarget();

    //distance to current target, defaults to max scan radius if no target exists
    const float DistToTarget = Target
        ? FVector::Dist(Spider->GetActorLocation(), Target->GetActorLocation())
        : ThreatScanRadius;

    //checks if the current target is entangled in a web
    bool bTargetEntangled = false;

    if (Target)
    {
        ACharacter* TargetChar = Cast<ACharacter>(Target);

        if (TargetChar && TargetChar->GetCharacterMovement())
            bTargetEntangled = TargetChar->GetCharacterMovement()->MaxWalkSpeed <= 0.f;
    }

    //patrol utility favours low risk environments and high health
    float PatrolScore = (1.f - Risk) * 0.5f + HealthNorm * 0.3f;

    if (Target)
        PatrolScore *= 0.1f;

    //ranged attack utility, favours optimal distance engagements against mobile targets
    float RangedScore = 0.f;

    if (Target && Spider->bCanRangedAttack
        && DistToTarget >= MinRangedDistance
        && DistToTarget <= MaxRangedDistance
        && !bTargetEntangled)
    {
        //distance factor rewards staying near optimal ranged attack distance
        float DistFactor = 1.f - FMath::Abs(DistToTarget - OptimalRangedDistance) / MaxRangedDistance;

        RangedScore = FMath::Max(0.f, DistFactor) * (1.f - Risk * 0.3f);
    }

    //melee utility, favours close range combat and entangled targets
    const float EffectiveMeleeRange = Spider->AttackRange;
    float MeleeScore = 0.f;

    if (Target)
    {
        //approach factor increases as spider gets closer to the target
        float ApproachFactor = 1.f - FMath::Clamp(DistToTarget / ThreatScanRadius, 0.f, 1.f);

        //melee factor strongly favours targets already within attack range
        float MeleeFactor = DistToTarget < EffectiveMeleeRange
            ? 1.f - DistToTarget / EffectiveMeleeRange
            : 0.f;

        float BaseScore = FMath::Max(ApproachFactor, MeleeFactor) * HealthNorm;

        //entangled targets receive a large melee priority bonus
        MeleeScore = bTargetEntangled ? FMath::Min(BaseScore * 2.5f, 1.f) : BaseScore;
    }

    //retreat utility increases with environmental risk and low health
    float RetreatScore = Risk * 0.6f + (1.f - HealthNorm) * 0.4f;

    //final weighted utility scores
    float WPatrol  = PatrolScore  * PatrolWeight;
    float WRanged  = RangedScore  * RangedWeight;
    float WMelee   = MeleeScore   * MeleeWeight;
    float WRetreat = RetreatScore * RetreatWeight;

    //defaults to patrol state, replaced if a higher utility state is found
    ESpiderState NewState = ESpiderState::Patrol;
    float Best = WPatrol;

    if (WRanged  > Best) { Best = WRanged;  NewState = ESpiderState::RangedAttack; }
    if (WMelee   > Best) { Best = WMelee;   NewState = ESpiderState::MeleeAttack;  }
    if (WRetreat > Best) { Best = WRetreat; NewState = ESpiderState::Retreat;      }

    //resets defensive web placement once retreat ends
    if (NewState != ESpiderState::Retreat)
        bWebPlacedThisRetreat = false;

    //stores the selected highest scoring state
    CurrentState = NewState;

    //executes behaviour logic for the chosen state
    switch (CurrentState)
    {
    case ESpiderState::Patrol:
    {
        //checks distance to current patrol destination
        const float DistToDest = FVector::Dist(Spider->GetActorLocation(), PatrolDestination);

        //generates a new random reachable patrol destination if required
        if (!bHasPatrolDestination || DistToDest < PatrolArrivalRadius)
        {
            UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
            FNavLocation NavLoc;

            if (NavSys && NavSys->GetRandomReachablePointInRadius(
                Spider->GetActorLocation(), PatrolRadius, NavLoc))
            {
                PatrolDestination     = NavLoc.Location;
                bHasPatrolDestination = true;
            }
        }

        //moves towards patrol destination if not already moving
        if (bHasPatrolDestination && GetMoveStatus() != EPathFollowingStatus::Type::Moving)
            MoveToLocation(PatrolDestination, 5.f);

        break;
    }

    case ESpiderState::RangedAttack:

        if (Target)
        {
            //maintains optimal ranged attack distance from the target
            if (DistToTarget > OptimalRangedDistance + 100.f)
                MoveToActor(Target, OptimalRangedDistance);
            else
                StopMovement();

            //fires a web projectile at the target
            Spider->ShootWebProjectile(Target);
        }

        break;

    case ESpiderState::MeleeAttack:

        if (Target)
        {
            //moves directly towards the target for melee combat
            MoveToActor(Target, 100.f);

            //stores current melee attack target
            Spider->CurrentAttackTarget = Target;

            //performs attack once inside attack range
            if (DistToTarget <= Spider->AttackRange)
                Spider->PerformAttack();
        }

        break;

    case ESpiderState::Retreat:
    {
        //calculates flee location away from current threat
        FVector ThreatLoc = Target
            ? Target->GetActorLocation()
            : Spider->GetActorLocation();

        MoveToLocation(CalcFleeDestination(ThreatLoc), 50.f);

        //places a defensive web once per retreat cycle
        if (!bWebPlacedThisRetreat)
        {
            Spider->PlaceDefensiveWeb();
            bWebPlacedThisRetreat = true;
        }

        break;
    }
    }
}