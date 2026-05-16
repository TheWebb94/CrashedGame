#include "BTService_BeeFlocking.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Crashed/NPC/Bee/Bee.h"
#include "Crashed/NPC/Bee/BeeHive.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTService_BeeFlocking::UBTService_BeeFlocking()
{
    NodeName = TEXT("BeeFlocking");
    Interval = 0.05f;
    RandomDeviation = 0.01f;
}

void UBTService_BeeFlocking::TickNode(UBehaviorTreeComponent& OwnerComp,
                                       uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* Controller = OwnerComp.GetAIOwner();
    UBlackboardComponent* BB  = OwnerComp.GetBlackboardComponent();
    if (!Controller || !BB) return;

    ABee* Self = Cast<ABee>(Controller->GetPawn());
    if (!Self || Self->bIsDefending) return;

    const FVector SelfLoc = Self->GetActorLocation();

    if (Self->bIsSwarmMaster)
{
    if (BB->GetValueAsBool(TEXT("HasPollinateTarget"))) return;

    // Gather hive info for roam clamping
    FVector HiveLocation  = FVector::ZeroVector;
    float   MaxRoam       = 0.f;
    bool    bHasHive      = Self->OwnerHive.IsValid();
    if (bHasHive)
    {
        HiveLocation = Self->OwnerHive->GetActorLocation();
        MaxRoam      = Self->OwnerHive->MaxRoamDistance;
    }

    // If already outside the boundary, head straight back
    if (bHasHive && MaxRoam > 0.f &&
        FVector::Dist(SelfLoc, HiveLocation) > MaxRoam)
    {
        BB->SetValueAsVector(TEXT("TargetLocation"), HiveLocation);
        Self->WanderTarget = FVector::ZeroVector; // reset so next pick is fresh
        return;
    }

    const float Now          = GetWorld()->GetTimeSeconds();
    const float DistToTarget = FVector::Dist(SelfLoc, Self->WanderTarget);
    if (Self->WanderTarget.IsZero() ||
        DistToTarget < 200.f ||
        (Now - Self->LastWanderUpdate) >= Self->WanderUpdateInterval)
    {
        const FVector Forward     = Self->GetActorForwardVector();
        const float   ForwardBias = FMath::FRandRange(-60.f, 60.f);
        const float   Yaw         = FMath::Atan2(Forward.Y, Forward.X)
                                    + FMath::DegreesToRadians(ForwardBias);
        const float   WanderDist  = FMath::FRandRange(400.f, Self->WanderRadius * 0.5f);
        FVector Candidate = SelfLoc + FVector(FMath::Cos(Yaw) * WanderDist,
                                               FMath::Sin(Yaw) * WanderDist, 0.f);

        // Clamp candidate so it stays within MaxRoamDistance of hive
        if (bHasHive && MaxRoam > 0.f)
        {
            FVector ToCandidate = Candidate - HiveLocation;
            if (ToCandidate.Size() > MaxRoam)
                Candidate = HiveLocation + ToCandidate.GetSafeNormal() * MaxRoam;
        }

        Self->WanderTarget     = Candidate;
        Self->LastWanderUpdate = Now;
    }

    BB->SetValueAsVector(TEXT("TargetLocation"), Self->WanderTarget);
    return;
}

    // Follower bees: track the swarm master
    if (Self->OwnerHive.IsValid())
    {
        if (ABee* Master = Self->OwnerHive->GetSwarmMaster())
        {
            const FVector ToSelf      = (SelfLoc - Master->GetActorLocation()).GetSafeNormal();
            const FVector GatherPoint = Master->GetActorLocation() + ToSelf;
            BB->SetValueAsVector(TEXT("TargetLocation"), GatherPoint);

            const float DistToGather = FVector::Dist(SelfLoc, GatherPoint);
            const float Alpha        = FMath::Clamp((DistToGather - 150.f) / 600.f, 0.f, 1.f);
            Self->GetCharacterMovement()->MaxWalkSpeed = FMath::Lerp(Self->MoveSpeed, Self->MoveSpeed * 2.5f, Alpha);

            return;
        }
    }

    // Fallback (no hive / no master): original flocking
    const float RadiusSq = Self->FlockingRadius * Self->FlockingRadius;

    TArray<AActor*> AllBees;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABee::StaticClass(), AllBees);

    FVector Separation  = FVector::ZeroVector;
    FVector CohesionSum = FVector::ZeroVector;
    FVector AlignSum    = FVector::ZeroVector;
    int32   Count       = 0;

    for (AActor* Actor : AllBees)
    {
        ABee* Other = Cast<ABee>(Actor);
        if (!Other || Other == Self) continue;

        const float DistSq = FVector::DistSquared(SelfLoc, Other->GetActorLocation());
        if (DistSq > RadiusSq) continue;

        const float   Dist    = FMath::Sqrt(DistSq);
        const FVector ToOther = Other->GetActorLocation() - SelfLoc;

        if (Dist < Self->DesiredSeparation && Dist > KINDA_SMALL_NUMBER)
            Separation -= ToOther.GetSafeNormal() * (Self->DesiredSeparation - Dist);

        CohesionSum += Other->GetActorLocation();

        const FVector OtherVel = Other->GetVelocity();
        if (!OtherVel.IsNearlyZero())
            AlignSum += OtherVel.GetSafeNormal();

        ++Count;
    }

    FVector Desired = FVector::ZeroVector;
    if (Count > 0)
    {
        FVector CohesionDir = ((CohesionSum / (float)Count) - SelfLoc).GetSafeNormal();
        FVector AlignDir    = (AlignSum / (float)Count).GetSafeNormal();
        Desired = Separation   * Self->SeparationWeight
                + CohesionDir * Self->CohesionWeight
                + AlignDir    * Self->AlignmentWeight;
    }

    if (!Desired.IsNearlyZero())
    {
        BB->SetValueAsVector(TEXT("TargetLocation"), SelfLoc + Desired.GetSafeNormal() * 200.f);
    }
    else
    {
        const float Now = GetWorld()->GetTimeSeconds();
        if (Self->WanderTarget.IsZero() ||
            (Now - Self->LastWanderUpdate) >= Self->WanderUpdateInterval)
        {
            const float Angle = FMath::FRandRange(0.f, 2.f * PI);
            const float Dist  = FMath::FRandRange(200.f, Self->WanderRadius * 0.4f);
            Self->WanderTarget = SelfLoc + FVector(
                FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0.f);
            Self->LastWanderUpdate = Now;
        }
        BB->SetValueAsVector(TEXT("TargetLocation"), Self->WanderTarget);
    }
}