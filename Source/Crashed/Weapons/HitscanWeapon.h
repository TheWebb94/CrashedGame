#pragma once
#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "HitscanWeapon.generated.h"

class UNiagaraSystem;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FWeaponFired, FVector, BeamStart, FVector, BeamEnd, bool, bHit);

UCLASS()
class CRASHED_API AHitscanWeapon : public ABaseWeapon
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& Origin, const FVector& AimPoint) override;
	AHitscanWeapon();

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float Damage = 25.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|VFX")
	TObjectPtr<UNiagaraSystem> BeamEffect;
	
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FWeaponFired WeaponFired;
};