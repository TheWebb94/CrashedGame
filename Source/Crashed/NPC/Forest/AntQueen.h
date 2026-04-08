#pragma once

#include "CoreMinimal.h"
#include "Crashed/NPC/BaseEnemy.h"
#include "AntQueen.generated.h"

class AForestAnt;

UCLASS(Abstract)
class CRASHED_API AAntQueen : public ABaseEnemy
{
	GENERATED_BODY()

public:
	AAntQueen();

	// Called by ants when they die and removes from tracking arrays
	UFUNCTION()
	void OnAntDied(ABaseEnemy* DeadAnt);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Queen|Spawning")
	TSubclassOf<AForestAnt> WorkerAntClass;

	UPROPERTY(EditDefaultsOnly, Category = "Queen|Spawning")
	TSubclassOf<AForestAnt> SoldierAntClass;

	UPROPERTY(EditDefaultsOnly, Category = "Queen|Spawning")
	int32 MaxWorkers = 10;

	UPROPERTY(EditDefaultsOnly, Category = "Queen|Spawning")
	int32 MaxSoldiers = 15;

	// Target number of workers per one soldier
	UPROPERTY(EditDefaultsOnly, Category = "Queen|Spawning")
	float WorkerToSoldierRatio = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Queen|Spawning")
	float SpawnInterval = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Queen|Spawning")
	float SpawnRadius = 400.0f;

	// How many soldiers to emergency-spawn when the queen is damaged
	UPROPERTY(EditDefaultsOnly, Category = "Queen|Spawning")
	int32 EmergencySoldierCount = 5;

private:
	UPROPERTY()
	TArray<TWeakObjectPtr<AForestAnt>> AliveWorkers;

	UPROPERTY()
	TArray<TWeakObjectPtr<AForestAnt>> AliveSoldiers;

	FTimerHandle SpawnTimerHandle;

	void CheckAndSpawn();
	void SpawnAnt(TSubclassOf<AForestAnt> AntClass, TArray<TWeakObjectPtr<AForestAnt>>& TrackingArray);
	void PurgeDeadEntries();

	UFUNCTION()
	void OnQueenDamaged(float NewHealth, float MaxHealth);
};
