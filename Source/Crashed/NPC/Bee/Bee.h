#pragma once

#include "CoreMinimal.h"
#include "Crashed/NPC/BaseEnemy.h"
#include "Bee.generated.h"

class AFoodSpawner;
class ABeeHive;
class USphereComponent;

UCLASS(Abstract)
class CRASHED_API ABee : public ABaseEnemy
{
	GENERATED_BODY()

public:
	ABee();

	UPROPERTY(BlueprintReadOnly, Category = "Bee")
	TWeakObjectPtr<ABeeHive> OwnerHive;

	UPROPERTY(BlueprintReadWrite, Category = "Bee|Combat")
	bool bIsDefending = false;

	UPROPERTY(BlueprintReadOnly, Category = "Bee")
	bool bIsSwarmMaster = false;

	float PreviousHealth = -1.f;

	TMap<TWeakObjectPtr<AFoodSpawner>, float> PollinationCooldowns;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bee|Separation")
	USphereComponent* SeparationSphere;

	UPROPERTY(EditDefaultsOnly, Category = "Bee|Separation")
	float SeparationSphereRadius = 250.f;

	UPROPERTY(EditDefaultsOnly, Category = "Bee|Separation")
	float SeparationStrength = 600.f;

	UPROPERTY(EditDefaultsOnly, Category = "Bee|Flocking")
	float FlockingRadius = 400.f;

	UPROPERTY(EditDefaultsOnly, Category = "Bee|Flocking")
	float SeparationWeight = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Bee|Flocking")
	float CohesionWeight = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Bee|Flocking")
	float AlignmentWeight = 0.8f;

	UPROPERTY(EditDefaultsOnly, Category = "Bee|Flocking")
	float DesiredSeparation = 60.f;

	UPROPERTY(EditDefaultsOnly, Category = "Bee|Flocking")
	float WanderRadius = 1500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Bee|Flocking")
	float WanderUpdateInterval = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "Bee|Flocking")
	float WanderWeight = 1.2f;

	FVector WanderTarget = FVector::ZeroVector;
	float   LastWanderUpdate = -999.f;

	UPROPERTY(EditDefaultsOnly, Category = "Bee|Pollination")
	float PollinationScanRadius = 3000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Bee|Pollination")
	float PollinationCooldown = 300.f;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void PerformAttack_Implementation() override;

	UFUNCTION()
	void OnBeeHealthChanged(float NewHealth, float MaxHealth);
};