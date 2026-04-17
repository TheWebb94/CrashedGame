#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AntHive.generated.h"

class UStaticMeshComponent;
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
};
