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

UCLASS(Abstract)
class CRASHED_API AForestAnt : public ABaseEnemy
{
	GENERATED_BODY()

public:
	AForestAnt();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ant")
	EAntType AntType = EAntType::Worker;

	// Set by the queen after spawning — lets the AI controller redirect movement
	TWeakObjectPtr<AAntHive> HomeHive;
	void SetHomeHive(AAntHive* InHive);

	// True while the ant is walking back to the hive to be healed
	bool bReturningHome = false;

	// Called by the hive when the ant has been fully healed
	void CompleteHealing();

	// Called by the queen after spawning
	void RegisterWithQueen(AAntQueen* InQueen);

	// --- Healer-only properties (ignored for other types) ---
	UPROPERTY(EditDefaultsOnly, Category = "Ant|Healing")
	float HealAmount = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "Ant|Healing")
	float HealRadius = 350.f;

	UPROPERTY(EditDefaultsOnly, Category = "Ant|Healing")
	float HealInterval = 2.f;

protected:
	virtual void BeginPlay() override;
	virtual void OnDeath_Implementation() override;
	virtual void PerformAttack_Implementation() override;


	// Health % below which this ant turns back to the hive.
	// Set automatically in BeginPlay per type; can be overridden in Blueprint.
	UPROPERTY(EditDefaultsOnly, Category = "Ant|Healing")
	float ReturnHealthThreshold = 0.25f;

private:
	TWeakObjectPtr<AAntQueen> Queen;

	// Stored when bReturningHome becomes true; restored by CompleteHealing()
	float RestoredDetectionRadius = 0.f;

	FTimerHandle HealTimerHandle;

	UFUNCTION()
	void OnAntHealthChanged(float NewHealth, float MaxHealth);

	void HealNearbyAnts();
};