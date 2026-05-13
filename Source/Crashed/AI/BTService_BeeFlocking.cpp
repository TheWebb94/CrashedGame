#include "BTService_BeeFlocking.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Crashed/NPC/Bee/Bee.h"
#include "Kismet/GameplayStatics.h"


UBTService_BeeFlocking::UBTService_BeeFlocking()
{
    NodeName = TEXT("BeeFlocking");
    Interval = 0.1f;
    RandomDeviation = 0.02f;
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
    const float   RadiusSq = Self->FlockingRadius * Self->FlockingRadius;

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

        const float Dist    = FMath::Sqrt(DistSq);
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

        Desired = Separation    * Self->SeparationWeight
                + CohesionDir  * Self->CohesionWeight
                + AlignDir     * Self->AlignmentWeight;
    }

    // Gentle upward bias to keep bees airborne
    Desired += FVector(0.f, 0.f, 30.f);

    if (!Desired.IsNearlyZero())
    {
        const FVector FlockTarget = SelfLoc + Desired.GetSafeNormal() * 200.f;
        BB->SetValueAsVector(TEXT("TargetLocation"), FlockTarget);
    }
}