#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseWeapon.generated.h"

UCLASS(Abstract)
class CRASHED_API ABaseWeapon : public AActor
{
	GENERATED_BODY()
public:
	ABaseWeapon();
	virtual void Fire(const FVector& Origin, const FVector& AimPoint) {}
	// Returns the static mesh assigned to this weapon (used to swap the player's TurretMesh)
	UStaticMesh* GetWeaponStaticMesh() const
	{
		return WeaponMesh ? WeaponMesh->GetStaticMesh() : nullptr;
	}
	
	UPROPERTY(EditDefaultsOnly, Category = "Display")
	float MeshYawOffset = 0.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Display")
	float MeshRollOffset = 0.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Display")
	FVector MeshScale = FVector(1.f, 1.f, 1.f);

	// Whether holding Fire continuously shoots
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	bool bAutoFire = false;

	// Rounds per second when bAutoFire is true
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float FireRate = 10.f;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UStaticMeshComponent* WeaponMesh;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	float Range = 2000.f;
};
