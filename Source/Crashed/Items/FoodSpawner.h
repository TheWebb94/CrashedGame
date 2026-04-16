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

protected:
	virtual void BeginPlay() override;

private:
	void SpawnFood();
	void RefreshFood();

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBillboardComponent* Marker;

	// Blueprint subclass of AFood to spawn (assign BP_Food in editor)
	UPROPERTY(EditAnywhere, Category = "Food Spawner")
	TSubclassOf<AFood> FoodClass;

	// Radius around this actor in which food is randomly placed
	UPROPERTY(EditAnywhere, Category = "Food Spawner")
	float SpawnRadius = 500.f;
	
	// Radius around this actor in which food is randomly placed
	UPROPERTY(EditAnywhere, Category = "Food Spawner")
	float minSpawnRadius = 150.f;

	// Maximum food items active at one time
	UPROPERTY(EditAnywhere, Category = "Food Spawner")
	int32 MaxFoodCount = 5;

	// Seconds between replenishment checks
	UPROPERTY(EditAnywhere, Category = "Food Spawner")
	float SpawnInterval = 15.f;

	TArray<TWeakObjectPtr<AFood>> ActiveFood;
	FTimerHandle SpawnTimerHandle;
};
