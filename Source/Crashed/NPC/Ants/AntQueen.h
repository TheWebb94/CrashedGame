#pragma once

#include "CoreMinimal.h"
#include "Crashed/NPC/BaseEnemy.h"
#include "AntQueen.generated.h"

class AForestAnt;
class AAntHive;

UCLASS(Abstract)
class CRASHED_API AAntQueen : public ABaseEnemy
{
    GENERATED_BODY()

public:
    AAntQueen();

    // Called by ants (via OnEnemyDeath delegate) when they die
    UFUNCTION()
    void OnAntDied(ABaseEnemy* DeadAnt);

protected:
    virtual void BeginPlay() override;

    // --- Worker / Soldier spawning ---
    UPROPERTY(EditDefaultsOnly, Category = "Queen|Spawning")
    TSubclassOf<AForestAnt> WorkerAntClass;

    UPROPERTY(EditDefaultsOnly, Category = "Queen|Spawning")
    TSubclassOf<AForestAnt> SoldierAntClass;

    UPROPERTY(EditDefaultsOnly, Category = "Queen|Spawning")
    int32 MaxWorkers = 10;

    UPROPERTY(EditDefaultsOnly, Category = "Queen|Spawning")
    int32 MaxSoldiers = 15;

    UPROPERTY(EditDefaultsOnly, Category = "Queen|Spawning")
    float WorkerToSoldierRatio = 2.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Queen|Spawning")
    float SpawnInterval = 10.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Queen|Spawning")
    float SpawnRadius = 400.0f;

    // --- Healer management ---
    UPROPERTY(EditDefaultsOnly, Category = "Queen|Healers")
    TSubclassOf<AForestAnt> HealerAntClass;

    UPROPERTY(EditDefaultsOnly, Category = "Queen|Healers")
    int32 InitialHealerCount = 3;

    // How long (seconds) before the queen replaces a dead healer
    UPROPERTY(EditDefaultsOnly, Category = "Queen|Healers")
    float HealerReplaceDelay = 15.f;

    // --- Phase triggers ---
    // Soldiers spawned when queen drops to 66% HP
    UPROPERTY(EditDefaultsOnly, Category = "Queen|Phases")
    int32 SoldiersSpawnedAt66 = 3;

    // Soldiers spawned when queen drops to 33% HP
    UPROPERTY(EditDefaultsOnly, Category = "Queen|Phases")
    int32 SoldiersSpawnedAt33 = 6;

    // --- Hive reference (set in editor) ---
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

    bool bThreshold66Triggered = false;
    bool bThreshold33Triggered = false;

    void CheckAndSpawn();
    void SpawnAnt(TSubclassOf<AForestAnt> AntClass, TArray<TWeakObjectPtr<AForestAnt>>& TrackingArray);
    void PurgeDeadEntries();
    void SpawnInitialHealers();
    void ScheduleHealerReplacement();

    UFUNCTION()
    void SpawnReplacementHealer();

    UFUNCTION()
    void OnQueenDamaged(float NewHealth, float MaxHealth);
};