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

    // --- Perception ---
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

    // --- Utility AI tuning ---
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

    // Weights (coefficients from the slides — higher = more dominant)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Utility|Weights")
    float PatrolWeight  = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Utility|Weights")
    float RangedWeight  = 2.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Utility|Weights")
    float MeleeWeight   = 1.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Utility|Weights")
    float RetreatWeight = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Utility")
    float DecisionInterval = 0.25f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
    ESpiderState CurrentState = ESpiderState::Patrol;

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* MyPawn) override;
    virtual FRotator GetControlRotation() const override;

private:
    // Re-evaluates utility scores and executes the winning action
    void EvaluateAndExecute();

    float CalculateRiskScore() const;
    AActor* FindBestTarget() const;

    // Flee-point calculation for retreat
    FVector CalcFleeDestination(const FVector& ThreatOrigin) const;

    // Tracks whether a defensive web was already placed this retreat phase
    bool bWebPlacedThisRetreat = false;

    FTimerHandle DecisionTimerHandle;
};