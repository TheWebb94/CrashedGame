#include "BTTask_HealAlly.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Crashed/NPC/Ants/ForestAnt.h"
#include "Crashed/HealthComponent.h"

UBTTask_HealAlly::UBTTask_HealAlly()
{
    NodeName = TEXT("HealAlly");
    bCreateNodeInstance = true;
}

void UBTTask_HealAlly::ClearBBKeys(UBlackboardComponent* BB)
{
    if (!BB) return;
    BB->ClearValue(TEXT("HealTarget"));
    BB->SetValueAsBool(TEXT("HasHealTarget"), false);
}

EBTNodeResult::Type UBTTask_HealAlly::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* Controller = OwnerComp.GetAIOwner();
    UBlackboardComponent* BB  = OwnerComp.GetBlackboardComponent();
    if (!Controller || !BB) return EBTNodeResult::Failed;

    APawn* Self = Controller->GetPawn();
    if (!Self) return EBTNodeResult::Failed;

    AForestAnt* Target = Cast<AForestAnt>(BB->GetValueAsObject(TEXT("HealTarget")));
    if (!IsValid(Target) || !Target->HealthComponent) { ClearBBKeys(BB); return EBTNodeResult::Failed; }
    if (Target->HealthComponent->GetHealthPercent() >= 1.f) { ClearBBKeys(BB); return EBTNodeResult::Succeeded; }
    if (FVector::Dist(Self->GetActorLocation(), Target->GetActorLocation()) > HealRange)
        return EBTNodeResult::Failed;

    Target->HealthComponent->ApplyHealth(HealAmount);
    if (Target->HealthComponent->GetHealthPercent() >= 1.f) { ClearBBKeys(BB); return EBTNodeResult::Succeeded; }

    CachedOwnerComp = &OwnerComp;
    const float Interval = (HealRate > 0.f) ? (1.f / HealRate) : 1.f;
    Self->GetWorldTimerManager().SetTimer(HealTimerHandle, this, &UBTTask_HealAlly::DoHealTick, Interval, true);

    return EBTNodeResult::InProgress;
}

void UBTTask_HealAlly::DoHealTick()
{
    if (!CachedOwnerComp) return;

    AAIController* Controller = CachedOwnerComp->GetAIOwner();
    UBlackboardComponent* BB  = CachedOwnerComp->GetBlackboardComponent();
    if (!Controller || !BB) { CachedOwnerComp = nullptr; return; }

    APawn* Self = Controller->GetPawn();

    AForestAnt* Target = Cast<AForestAnt>(BB->GetValueAsObject(TEXT("HealTarget")));

    // Target gone or dead — give up entirely
    if (!IsValid(Target) || !Target->HealthComponent)
    {
        ClearBBKeys(BB);
        if (Self) Self->GetWorldTimerManager().ClearTimer(HealTimerHandle);
        FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Failed);
        CachedOwnerComp = nullptr;
        return;
    }

    // Target walked out of range — stop healing, let BT re-run MoveTo to chase them
    if (Self && FVector::Dist(Self->GetActorLocation(), Target->GetActorLocation()) > HealRange)
    {
      
        Self->GetWorldTimerManager().ClearTimer(HealTimerHandle);
        FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Failed);
        CachedOwnerComp = nullptr;
        return;
    }

    Target->HealthComponent->ApplyHealth(HealAmount);

    if (Target->HealthComponent->GetHealthPercent() >= 1.f)
    {
        ClearBBKeys(BB);
        if (Self) Self->GetWorldTimerManager().ClearTimer(HealTimerHandle);
        FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
        CachedOwnerComp = nullptr;
    }
}

EBTNodeResult::Type UBTTask_HealAlly::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (AAIController* Controller = OwnerComp.GetAIOwner())
        if (APawn* P = Controller->GetPawn())
            P->GetWorldTimerManager().ClearTimer(HealTimerHandle);

    ClearBBKeys(OwnerComp.GetBlackboardComponent());
    CachedOwnerComp = nullptr;
    return EBTNodeResult::Aborted;
}
