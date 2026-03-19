#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponProjectile.generated.h"

UCLASS()
class CRASHED_API AWeaponProjectile : public AActor
{
	GENERATED_BODY()
public:
	AWeaponProjectile();
	virtual void Tick(float DeltaTime) override;

	// Called by ProjectileWeapon immediately after spawning
	void Launch(const FVector& InDirection, float InTravelDistance);
	
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float Speed = 1500.f;
	
	UPROPERTY(EditAnywhere, Category = "Projectile")
	float Damage = 25.f;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float ExplosionRadius = 200.f;
	
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
			   UPrimitiveComponent* OtherComp, FVector NormalImpulse,
			   const FHitResult& Hit);

protected:
	UPROPERTY(VisibleAnywhere) class USphereComponent* CollisionComp;
	UPROPERTY(VisibleAnywhere) class UStaticMeshComponent* ProjectileMesh;
	
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float MeshScale = 0.001f;

private:
	FVector Direction;
	float TravelDistance = 0.f;
	float DistanceTraveled = 0.f;
	bool bLaunched = false;

	void Explode();
};
