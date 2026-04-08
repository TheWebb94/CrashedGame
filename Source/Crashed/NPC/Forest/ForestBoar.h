#pragma once

#include "CoreMinimal.h"
#include "Crashed/NPC/BaseEnemy.h"
#include "ForestBoar.generated.h"

class USphereComponent;

UCLASS(Abstract)
class CRASHED_API AForestBoar : public ABaseEnemy
{
	GENERATED_BODY()

public:
	AForestBoar();

protected:
	virtual void BeginPlay() override;

	// Sphere radius that triggers aggro — set separately from AI detection radius
	UPROPERTY(EditDefaultsOnly, Category = "Boar")
	float AggroRadius = 500.f;

	// Detection radius used while aggravated
	UPROPERTY(EditDefaultsOnly, Category = "Boar")
	float HostileDetectionRadius = 2500.f;

	// Seconds after player leaves AggroZone before the boar calms down
	UPROPERTY(EditDefaultsOnly, Category = "Boar")
	float AggroDecayTime = 6.f;

	UPROPERTY(BlueprintReadOnly, Category = "Boar")
	bool bIsAggravated = false;

private:
	UPROPERTY(VisibleAnywhere)
	USphereComponent* AggroZone;

	float PassiveDetectionRadius = 0.f;
	FTimerHandle AggroDecayTimer;

	UFUNCTION()
	void OnAggroZoneBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnAggroZoneEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void BecomeAggravated();
	void ResetAggro();
};
