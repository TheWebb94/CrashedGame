#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpiderWebProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;

UCLASS()
class CRASHED_API ASpiderWebProjectile : public AActor
{
	GENERATED_BODY()

public:
	ASpiderWebProjectile();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* WebMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;

	// How long the hit target is frozen (seconds)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Web")
	float EntangleDuration = 3.0f;
	//travel speed of projectile
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Web")
	float ProjectileSpeed = 800.0f;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
				   UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
				   bool bFromSweep, const FHitResult& SweepResult);

	void EntangleTarget(AActor* Target);
};