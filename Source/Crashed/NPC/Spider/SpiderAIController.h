#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SpiderAIController.generated.h"

class UAISenseConfig_Sight;

UENUM()
enum class ESpiderState : uint8
{
    Patrol,
    RangedAttack,
    MeleeAttack,
    Retreat
};

UCLASS()
class CRASHED_API ASpiderAIController : public AAIController
{
    GENERATED_BODY()

public:
    ASpiderAIController();

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI|Perception")
    float AISightRadius = 1000.f;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI|Perception")
    float AISightAge = 5.f;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI|Perception")
    float AILoseSightRadius = 1050.f;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI|Perception")
    float AIFieldOfView = 130.f;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI|Perception")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Utility")
    float ThreatScanRadius = 1200.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Utility")
    float OptimalRangedDistance = 600.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Utility")
    float MinRangedDistance = 300.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Utility")
    float MaxRangedDistance = 1000.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Utility")
    float MeleeRange = 250.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Utility|Weights")
    float PatrolWeight  = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Utility|Weights")
    float RangedWeight  = 2.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Utility|Weights")
    float MeleeWeight   = 1.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Utility|Weights")
    float RetreatWeight = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Utility") //sets how often the unit evaluates its state. This number will affect sim performance a fair amount the lower it gets
    float DecisionInterval = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Patrol")
    float PatrolRadius = 800.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Patrol")
    float PatrolArrivalRadius = 120.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
    ESpiderState CurrentState = ESpiderState::Patrol;

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* MyPawn) override;
    virtual FRotator GetControlRotation() const override;

private:
    void EvaluateAndExecute();
    float CalculateRiskScore() const;
    AActor* FindBestTarget() const;
    FVector CalcFleeDestination(const FVector& ThreatOrigin) const;

    bool bWebPlacedThisRetreat    = false;
    FVector PatrolDestination     = FVector::ZeroVector;
    bool    bHasPatrolDestination = false;

    FTimerHandle DecisionTimerHandle;
};