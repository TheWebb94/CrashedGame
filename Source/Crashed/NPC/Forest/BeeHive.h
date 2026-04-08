#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BeeHive.generated.h"

class UHealthComponent;
class AForestBee;
class ABaseEnemy;

UCLASS()
class CRASHED_API ABeeHive : public AActor
{
	GENERATED_BODY()

public:
	ABeeHive();

	// Called by bees when they die — removes from AliveBees
	UFUNCTION()
	void OnBeeDied(ABaseEnemy* DeadBee);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* HiveMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Hive")
	TSubclassOf<AForestBee> BeeClass;

	// Bees spawned per swarm trigger
	UPROPERTY(EditDefaultsOnly, Category = "Hive")
	int32 BeesPerSwarm = 5;

	UPROPERTY(EditDefaultsOnly, Category = "Hive")
	float BeeSpawnRadius = 200.f;

	// Minimum seconds between swarm spawns
	UPROPERTY(EditDefaultsOnly, Category = "Hive")
	float AlertCooldown = 5.f;

private:
	UPROPERTY()
	TArray<TWeakObjectPtr<AForestBee>> AliveBees;

	bool bAlerted = false;
	FTimerHandle AlertCooldownTimer;

	UFUNCTION()
	void OnHiveDamaged(float NewHealth, float MaxHealth);

	void SpawnSwarm();

	UFUNCTION()
	void ResetAlert();
};
