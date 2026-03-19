#pragma once
#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "HitscanWeapon.generated.h"

UCLASS()
class CRASHED_API AHitscanWeapon : public ABaseWeapon
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& Origin, const FVector& AimPoint) override;
};
