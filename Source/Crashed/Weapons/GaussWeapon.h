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
		bAutoFire = false;
		FireRate  = 2.f;
	}
};