#pragma once
#include "CoreMinimal.h"
#include "ProjectileWeapon.h"
#include "RocketWeapon.generated.h"

UCLASS()
class CRASHED_API ARocketWeapon : public AProjectileWeapon
{
	GENERATED_BODY()
public:
	ARocketWeapon()
	{
		bAutoFire    = false;
		FireRate     = 1.f;   // 1 rocket/sec max
		stopAtCursor = true;  // projectile detonates where the cursor was
	}
};