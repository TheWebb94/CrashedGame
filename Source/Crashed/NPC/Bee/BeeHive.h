#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BeeHive.generated.h"

class ABee;
class UHealthComponent;
class UStaticMeshComponent;
class ABaseEnemy;

UCLASS()
class CRASHED_API ABeeHive : public AActor
{
	GENERATED_BODY()

public:
	ABeeHive();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* HiveMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComponent;

	UPROPERTY(EditDefaultsOnly, Category = "BeeHive")
	TSubclassOf<ABee> BeeClass;

	UPROPERTY(EditDefaultsOnly, Category = "BeeHive")
	int32 MaxBees = 12;

	UPROPERTY(EditDefaultsOnly, Category = "BeeHive")
	float BeeRespawnDelay = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "BeeHive")
	float SpawnRadius = 300.f;

	// How long bees stay in defend mode after a hive attack before returning to swarm
	UPROPERTY(EditDefaultsOnly, Category = "BeeHive")
	float DefendDuration = 15.f;

	UPROPERTY(EditDefaultsOnly, Category = "BeeHive")
	float MaxRoamDistance = 5000.f;
	
	ABee* GetSwarmMaster() const { return SwarmMaster.IsValid() ? SwarmMaster.Get() : nullptr; }
	
	void SendBeesToAttack(AActor* Target);
	
private:
	
	TWeakObjectPtr<ABee> SwarmMaster;
	
	TArray<TWeakObjectPtr<ABee>> AliveBees;
	int32 PendingRespawns = 0;
	FTimerHandle DefendResetTimerHandle;

	void SpawnBee();
	void TryRespawnBee();
	
	void ResetDefendState();
	void PromoteSwarmMaster();
	
	UFUNCTION()
	void OnHiveDamaged(float NewHealth, float MaxHealth);

	UFUNCTION()
	void OnBeeDied(ABaseEnemy* DeadBee);
};