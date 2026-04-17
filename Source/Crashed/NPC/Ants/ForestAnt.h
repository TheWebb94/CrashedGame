#pragma once

#include "CoreMinimal.h"
#include "Crashed/NPC/BaseEnemy.h"
#include "ForestAnt.generated.h"

class AAntQueen;
class AAntHive;

UENUM(BlueprintType)
enum class EAntType : uint8
{
    Worker  UMETA(DisplayName = "Worker"),
    Soldier UMETA(DisplayName = "Soldier"),
    Healer  UMETA(DisplayName = "Healer")
};

UENUM(BlueprintType)
enum class ESoldierPatrolType : uint8
{
    RandomPatrol UMETA(DisplayName = "Random Patrol"),
    PathPatrol   UMETA(DisplayName = "Path Patrol")
};

UCLASS(Abstract)
class CRASHED_API AForestAnt : public ABaseEnemy
{
    GENERATED_BODY()

public:
    AForestAnt();

    void SetCarryingFood(bool bCarrying, float Amount = 0.f);
    bool  IsCarryingFood()       const { return bIsCarryingFood; }
    float GetCarriedFoodAmount() const { return CarriedFoodAmount; }

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ant")
    EAntType AntType = EAntType::Worker;

    UPROPERTY(BlueprintReadOnly, Category = "Ant")
    TWeakObjectPtr<AAntHive> HomeHive = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Ant|Soldier")
    ESoldierPatrolType SoldierPatrolType = ESoldierPatrolType::RandomPatrol;

    void SetHomeHive(AAntHive* InHive);
    void RegisterWithQueen(AAntQueen* InQueen);

    // Called by the queen to assign this soldier's patrol style
    void SetSoldierPatrolType(ESoldierPatrolType Type);

    // Called by the queen to make this soldier rush back to the hive
    void SetDefendHive(bool bDefend);

    // Called by the queen to assign an attack target to this soldier
    void SetAttackTarget(AActor* Target);

    // Radius within which nearby soldiers are alerted when this worker is attacked
    UPROPERTY(EditDefaultsOnly, Category = "Ant|Worker")
    float WorkerAlertRadius = 1000.f;

protected:
    virtual void BeginPlay() override;
    virtual void OnDeath_Implementation() override;
    virtual void PerformAttack_Implementation() override;

    TWeakObjectPtr<AAntQueen> Queen;

private:
    bool  bIsCarryingFood   = false;
    float CarriedFoodAmount = 0.f;
    float PreviousHealth    = -1.f;

    UFUNCTION()
    void OnWorkerHealthChanged(float NewHealth, float MaxHealth);

    void AlertNearbyThreat();
};
