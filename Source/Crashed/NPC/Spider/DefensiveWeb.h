#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DefensiveWeb.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class CRASHED_API ADefensiveWeb : public AActor
{
	GENERATED_BODY()

public:
	ADefensiveWeb();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* WebMesh;

	// How long units are entangled when stepping on the web
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Web")
	float EntangleDuration = 3.0f;

	// How long the web stays in the world before auto-destroying
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Web")
	float WebLifespan = 30.0f;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
						UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
						bool bFromSweep, const FHitResult& SweepResult);

	void EntangleTarget(AActor* Target);

	// Prevents re-triggering while an actor is already entangled
	TSet<AActor*> EntangledActors;
};