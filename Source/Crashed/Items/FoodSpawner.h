#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FoodSpawner.generated.h"

class AFood;
class UBillboardComponent;

UCLASS()
class CRASHED_API AFoodSpawner : public AActor
{
	GENERATED_BODY()

public:
	AFoodSpawner();

	// Called by a bee to mark this spawner as pollinated
	UFUNCTION(BlueprintCallable, Category = "Food Spawner|Pollination")
	void Pollinate();

	UFUNCTION(BlueprintPure, Category = "Food Spawner|Pollination")
	bool IsPollinated() const { return bIsPollinated; }

	UFUNCTION(BlueprintPure, Category = "Food Spawner|Pollination")
	bool RequiresPollination() const { return bRequiresPollination; }

protected:
	virtual void BeginPlay() override;

private:
	void SpawnFood();
	void RefreshFood();

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBillboardComponent* Marker;

	UPROPERTY(EditAnywhere, Category = "Food Spawner")
	TSubclassOf<AFood> FoodClass;

	UPROPERTY(EditAnywhere, Category = "Food Spawner")
	float SpawnRadius = 500.f;

	UPROPERTY(EditAnywhere, Category = "Food Spawner")
	float minSpawnRadius = 150.f;

	UPROPERTY(EditAnywhere, Category = "Food Spawner")
	int32 MaxFoodCount = 5;

	UPROPERTY(EditAnywhere, Category = "Food Spawner")
	float SpawnInterval = 15.f;

	// If true, bIsPollinated must be true before RefreshFood will spawn food
	UPROPERTY(EditAnywhere, Category = "Food Spawner|Pollination")
	bool bRequiresPollination = true;

	// Consumed (set false) after one replenishment cycle
	UPROPERTY(VisibleAnywhere, Category = "Food Spawner|Pollination")
	bool bIsPollinated = false;

	TArray<TWeakObjectPtr<AFood>> ActiveFood;
	FTimerHandle SpawnTimerHandle;
};