#pragma once
#include "CoreMinimal.h"
#include "ProjectileWeapon.h"
#include "MachineGunWeapon.generated.h"

UCLASS()
class CRASHED_API AMachineGunWeapon : public AProjectileWeapon
{
	GENERATED_BODY()
public:
	AMachineGunWeapon()
	{
		bAutoFire  = true;
		FireRate   = 20.f;    // x rounds/sec
		stopAtCursor = false; // bullets travel to Range -  not stopped at cursor
	}
};