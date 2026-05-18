#pragma once
#include "CoreMinimal.h"
#include "HitscanWeapon.h"
#include "GaussWeapon.generated.h"

UCLASS()
class CRASHED_API AGaussWeapon : public AHitscanWeapon
{
	GENERATED_BODY()
public:
	AGaussWeapon()
	{
		bAutoFire = false; //whether the weapon continues firing on mouse hold down
		FireRate  = 2.f;
	}
};