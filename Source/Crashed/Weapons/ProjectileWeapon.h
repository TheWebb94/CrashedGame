#pragma once
#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "ProjectileWeapon.generated.h"

UCLASS()
class CRASHED_API AProjectileWeapon : public ABaseWeapon
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& Origin, const FVector& AimPoint) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<class AWeaponProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	bool stopAtCursor = false;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	bool isBurstWeapon = true;
};
