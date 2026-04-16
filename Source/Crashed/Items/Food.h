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

	UFUNCTION(BlueprintCallable, Category = "Food")
	float TakeFood();

	UFUNCTION(BlueprintCallable, Category = "Food")
	void DecayFood();

	UFUNCTION(BlueprintPure, Category = "Food")
	bool IsAvailable() const { return !bDepleted; }

	UPROPERTY(BlueprintReadOnly, Category = "Food")
	float FoodAmount = 0.f;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* FoodMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* PickupRadius;

	//Three mesh stages — assign in the Blueprint defaults
	UPROPERTY(EditDefaultsOnly, Category = "Food|Meshes")
	UStaticMesh* MeshWhole;

	UPROPERTY(EditDefaultsOnly, Category = "Food|Meshes")
	UStaticMesh* MeshSlightlyEaten;

	UPROPERTY(EditDefaultsOnly, Category = "Food|Meshes")
	UStaticMesh* MeshMostlyEaten;

	//Starting value range
	UPROPERTY(EditDefaultsOnly, Category = "Food|Spawning")
	float MinStartAmount = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "Food|Spawning")
	float MaxStartAmount = 30.f;

	//Visual scale range (maps min→max amount to these scales)
	UPROPERTY(EditDefaultsOnly, Category = "Food|Spawning")
	float MinScale = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Food|Spawning")
	float MaxScale = 2.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Food")
	float DespawnDelay = 5.f;

	float decayRate = 0.92f;

private:
	bool  bDepleted        = false;
	float InitialFoodAmount = 0.f;

	void UpdateMesh();
};
