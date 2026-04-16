#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Food.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class CRASHED_API AFood : public AActor
{
	GENERATED_BODY()

public:
	AFood();

	// Depletes the food and schedules actor destruction. Returns 0 if already taken.
	UFUNCTION(BlueprintCallable, Category = "Food")
	float TakeFood();
	
	UFUNCTION(BlueprintCallable, Category = "Food")
	void DecayFood();

	UFUNCTION(BlueprintPure, Category = "Food")
	bool IsAvailable() const { return !bDepleted; }
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Food")
	float FoodAmount = 10.f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* FoodMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* PickupRadius;

	
	UPROPERTY(EditDefaultsOnly, Category = "Food")
	float DespawnDelay = 10.f; // despawn delay will mean ants come back to checek decayed food for a period
	float decayRate = 0.75f;
	;

private:
	bool bDepleted = false;
};
