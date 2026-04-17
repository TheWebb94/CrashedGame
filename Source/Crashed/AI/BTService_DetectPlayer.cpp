#include "BTService_DetectPlayer.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

UBTService_DetectPlayer::UBTService_DetectPlayer()
{
    NodeName        = "DetectThreat";
    Interval        = 0.2f;
    RandomDeviation = 0.05f;
}

void UBTService_DetectPlayer::TickNode(UBehaviorTreeComponent& OwnerComp,
                                        uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* Controller = OwnerComp.GetAIOwner();
    UBlackboardComponent* BB  = OwnerComp.GetBlackboardComponent();
    if (!Controller || !BB) return;

    APawn* Pawn = Controller->GetPawn();
    if (!Pawn)  return;

    const FVector PawnLoc = Pawn->GetActorLocation();

    // Find the closest threat across all configured threat classes
    AActor* ClosestThreat   = nullptr;
    float   ClosestDistSq   = FLT_MAX;

    for (const TSubclassOf<AActor>& ThreatClass : ThreatClasses)
    {
        if (!ThreatClass) continue;

        TArray<AActor*> Found;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ThreatClass, Found);

        for (AActor* Actor : Found)
        {
            if (!Actor || Actor == Pawn) continue;

            const float DistSq = FVector::DistSquared(PawnLoc, Actor->GetActorLocation());
            if (DistSq < ClosestDistSq)
            {
                ClosestDistSq = DistSq;
                ClosestThreat = Actor;
            }
        }
    }

    const bool bCurrentlyFleeing = BB->GetValueAsBool(TEXT("IsPlayerNear"));

    if (!bCurrentlyFleeing && ClosestThreat && ClosestDistSq <= FleeRadius * FleeRadius)
    {
        BB->SetValueAsBool(TEXT("IsPlayerNear"), true);
        BB->SetValueAsVector(TEXT("ThreatLocation"), ClosestThreat->GetActorLocation());
    }
    else if (bCurrentlyFleeing && ClosestDistSq > SafeRadius * SafeRadius)
    {
        BB->SetValueAsBool(TEXT("IsPlayerNear"), false);
    }
    else if (bCurrentlyFleeing && ClosestThreat)
    {
        // Keep threat location pointed at the closest threat while still fleeing
        BB->SetValueAsVector(TEXT("ThreatLocation"), ClosestThreat->GetActorLocation());
    }
}
