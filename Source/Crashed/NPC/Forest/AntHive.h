#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AntHive.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class AForestAnt;
class AAntQueen;

UCLASS()
class CRASHED_API AAntHive : public AActor
{
	GENERATED_BODY()

public:
	AAntHive();

	// Set in editor — links hive to its queen so the queen can spawn near the hive
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Hive")
	AAntQueen* Queen;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* HiveMesh;

	// Ants walk into this sphere to receive healing
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* HealZone;

	// HP restored to each ant per tick
	UPROPERTY(EditDefaultsOnly, Category = "Hive|Healing")
	float HealTickAmount = 8.f;

	// Seconds between heal ticks
	UPROPERTY(EditDefaultsOnly, Category = "Hive|Healing")
	float HealTickInterval = 1.f;

private:
	UPROPERTY()
	TArray<TWeakObjectPtr<AForestAnt>> AntsInHealZone;

	FTimerHandle HealTickTimer;

	UFUNCTION()
	void OnHealZoneBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnHealZoneEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void HealTick();
};