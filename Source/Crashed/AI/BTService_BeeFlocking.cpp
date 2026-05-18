#include "BTService_BeeFlocking.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Crashed/NPC/Bee/Bee.h"
#include "Crashed/NPC/Bee/BeeHive.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

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
    //config
    AAIController* Controller = OwnerComp.GetAIOwner();
    UBlackboardComponent* BB  = OwnerComp.GetBlackboardComponent();
    if (!Controller || !BB) return;

    //cast guards
    ABee* Self = Cast<ABee>(Controller->GetPawn());
    if (!Self || Self->bIsDefending) return;

    const FVector SelfLoc = Self->GetActorLocation();

    //Swarm master (SM): wander freely within roam distance
    if (Self->bIsSwarmMaster) //one swarmmasteer is chosen, replaced on death
    {
        if (BB->GetValueAsBool(TEXT("HasPollinateTarget"))) return; //if the SM has pollinate target we dont want to patrol as pollination is higher priority

        FVector HiveLocation = FVector::ZeroVector;
        float   MaxRoam      = 0.f;
        bool    bHasHive     = Self->OwnerHive.IsValid();
        if (bHasHive)
        {
            HiveLocation = Self->OwnerHive->GetActorLocation();
            MaxRoam      = Self->OwnerHive->MaxRoamDistance;
        }
        
        //if hive is still valid, and current location is outside of the maximum roam from the hive, set the hive as the new destination
        if (bHasHive && MaxRoam > 0.f && FVector::Dist(SelfLoc, HiveLocation) > MaxRoam) 
        {
            BB->SetValueAsVector(TEXT("TargetLocation"), HiveLocation);
            Self->WanderTarget = FVector::ZeroVector;
            return;
        }

        const float Now          = GetWorld()->GetTimeSeconds();
        const float DistToTarget = FVector::Dist(SelfLoc, Self->WanderTarget);
        
        /*if we need a new target do x
        bees have a generally forwards bias in picking movement direction, 
        this was to counter a bug where a sharp turn casued massive traffic jams in the swarm */
        if (Self->WanderTarget.IsZero() || 
            DistToTarget < 200.f ||
            (Now - Self->LastWanderUpdate) >= Self->WanderUpdateInterval)
        {
            const FVector Forward     = Self->GetActorForwardVector(); //get actor facing direction
            const float   ForwardBias = FMath::FRandRange(-60.f, 60.f); //look forwards in a 120deg cone
            const float   Yaw         = FMath::Atan2(Forward.Y, Forward.X)
                                        + FMath::DegreesToRadians(ForwardBias);
            const float   WanderDist  = FMath::FRandRange(400.f, Self->WanderRadius * 0.5f); //pick a wander distance in this direction
            FVector Candidate = SelfLoc + FVector(FMath::Cos(Yaw) * WanderDist,
                                                   FMath::Sin(Yaw) * WanderDist, 0.f);

            if (bHasHive && MaxRoam > 0.f)
            {
                FVector ToCandidate = Candidate - HiveLocation; //check the candidate is within roam distance of the hive
                if (ToCandidate.Size() > MaxRoam)
                    Candidate = HiveLocation + ToCandidate.GetSafeNormal() * MaxRoam; //if too far, head that distyance until max raom dist reached
            }

            Self->WanderTarget     = Candidate;
            Self->LastWanderUpdate = Now;
        }

        BB->SetValueAsVector(TEXT("TargetLocation"), Self->WanderTarget); //pass value to BT
        return;
    }

    //Follower: track swarm master with speed boost by distance
    if (Self->OwnerHive.IsValid()) 
    {
        if (ABee* Master = Self->OwnerHive->GetSwarmMaster()) //access the SM data
        {
            const FVector ToSelf      = (SelfLoc - Master->GetActorLocation()).GetSafeNormal(); //find distance from self to SM
            const FVector GatherPoint = Master->GetActorLocation() + ToSelf * (Self->DesiredSeparation * 1.2f); //then look for a location near to SM
            BB->SetValueAsVector(TEXT("TargetLocation"), GatherPoint); // and travel to this location

            const float DistToGather = FVector::Dist(SelfLoc, GatherPoint); //check how far the target loc is
            const float Alpha        = FMath::Clamp((DistToGather - 150.f) / 600.f, 0.f, 1.f); 
            Self->GetCharacterMovement()->MaxWalkSpeed = FMath::Lerp(Self->MoveSpeed, Self->MoveSpeed * 1.5f, Alpha); //moderate the bee movement speed to allow catching up to the swarm, scale on dist

            return;
        }
    }

    //Fallback: full flocking (no hive or no master)
    const float RadiusSq = Self->FlockingRadius * Self->FlockingRadius;

    // create a list of all bees
    TArray<AActor*> AllBees;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABee::StaticClass(), AllBees);

    FVector CohesionSum = FVector::ZeroVector;
    FVector AlignSum    = FVector::ZeroVector;
    int32   Count       = 0;

    //loop through bee list and find out the relative locations of all bees
    for (AActor* Actor : AllBees)
    {
        ABee* Other = Cast<ABee>(Actor); //cast guard
        if (!Other || Other == Self) continue;

        const float DistSq = FVector::DistSquared(SelfLoc, Other->GetActorLocation());
        if (DistSq > RadiusSq) continue;

        CohesionSum += Other->GetActorLocation(); // summates bee locations

        const FVector OtherVel = Other->GetVelocity();
        if (!OtherVel.IsNearlyZero())
            AlignSum += OtherVel.GetSafeNormal(); //summatee bee velocities

        ++Count;
    }

    FVector Desired = FVector::ZeroVector;
    if (Count > 0)
    {
        FVector CohesionDir = ((CohesionSum / (float)Count) - SelfLoc).GetSafeNormal(); //finds average bee location
        FVector AlignDir    = (AlignSum / (float)Count).GetSafeNormal();                // and avereage alignemnt
        Desired = CohesionDir * Self->CohesionWeight                                    //then applies the averages modified by a weight
                + AlignDir    * Self->AlignmentWeight;
    }
    
    //if the above calc produces a meaningful value, set as target location
    if (!Desired.IsNearlyZero())
    {
        BB->SetValueAsVector(TEXT("TargetLocation"), SelfLoc + Desired.GetSafeNormal() * 200.f); 
    }
    else //if the above fails to producee a meaningful value, pick a random forward roam direction
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