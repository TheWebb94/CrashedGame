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
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
			   UPrimitiveComponent* OtherComponent, FVector NormalImpulse,
			   const FHitResult& Hit);

	void EntangleTarget(AActor* Target);
};