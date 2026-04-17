#pragma once

#include "CoreMinimal.h"
#include "Crashed/NPC/BaseEnemy.h"
#include "AntQueen.generated.h"

class AForestAnt;
class AAntHive;
class APatrolPath;

UCLASS(Abstract)
class CRASHED_API AAntQueen : public ABaseEnemy
{
    GENERATED_BODY()

public:
    AAntQueen();

    UFUNCTION(BlueprintCallable, Category = "Queen|Food")
    void ReceiveFood(float Amount);

    UFUNCTION()
    void OnAntDied(ABaseEnemy* DeadAnt);

    // Alerts soldiers within Radius of Origin to attack Target
    void AlertSoldiersNearby(FVector Origin, AActor* Target, float Radius);

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Queen|Patrol")
    APatrolPath* AntPatrolPath;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, Category = "Queen|Food")
    int32 MinWorkersBeforeFoodCost = 8;

    UPROPERTY(EditDefaultsOnly, Category = "Queen|Food")
    float FoodCostPerSpawn = 3.f;

    UPROPERTY(EditDefaultsOnly, Category = "Queen|Spawning")
    TSubclassOf<AForestAnt> WorkerAntClass;

    UPROPERTY(EditDefaultsOnly, Category = "Queen|Spawning")
    TSubclassOf<AForestAnt> SoldierAntClass;

    UPROPERTY(EditDefaultsOnly, Category = "Queen|Spawning")
    int32 MaxWorkers = 50;

    UPROPERTY(EditDefaultsOnly, Category = "Queen|Spawning")
    int32 MaxSoldiers = 15;

    UPROPERTY(EditDefaultsOnly, Category = "Queen|Spawning")
    float WorkerToSoldierRatio = 2.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Queen|Spawning")
    float SpawnInterval = 5.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Queen|Spawning")
    float SpawnRadius = 400.0f;

    // Fraction of soldiers assigned path patrol vs random patrol (0=all random, 1=all path)
    UPROPERTY(EditDefaultsOnly, Category = "Queen|Spawning")
    float PathPatrolSoldierRatio = 0.5f;

    UPROPERTY(EditDefaultsOnly, Category = "Queen|Healers")
    TSubclassOf<AForestAnt> HealerAntClass;

    UPROPERTY(EditDefaultsOnly, Category = "Queen|Healers")
    int32 InitialHealerCount = 3;

    UPROPERTY(EditDefaultsOnly, Category = "Queen|Healers")
    float HealerReplaceDelay = 15.f;

    UPROPERTY(EditDefaultsOnly, Category = "Queen|Phases")
    int32 SoldiersSpawnedAt66 = 3;

    UPROPERTY(EditDefaultsOnly, Category = "Queen|Phases")
    int32 SoldiersSpawnedAt33 = 6;

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Queen|Hive")
    AAntHive* HomeHive;

private:
    UPROPERTY()
    TArray<TWeakObjectPtr<AForestAnt>> AliveWorkers;

    UPROPERTY()
    TArray<TWeakObjectPtr<AForestAnt>> AliveSoldiers;

    UPROPERTY()
    TArray<TWeakObjectPtr<AForestAnt>> AliveHealers;

    FTimerHandle SpawnTimerHandle;

    bool  bThreshold66Triggered = false;
    bool  bThreshold33Triggered = false;
    float FoodSupply            = 0.f;

    void CheckAndSpawn();
    void SpawnAnt(TSubclassOf<AForestAnt> AntClass, TArray<TWeakObjectPtr<AForestAnt>>& TrackingArray);
    void PurgeDeadEntries();
    void SpawnInitialHealers();
    void ScheduleHealerReplacement();
    void NotifyAllSoldiersDefend();

    UFUNCTION()
    void SpawnReplacementHealer();

    UFUNCTION()
    void OnQueenDamaged(float NewHealth, float MaxHealth);
};
