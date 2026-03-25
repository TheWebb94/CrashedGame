#include "HitscanWeapon.h"
#include "DrawDebugHelpers.h"
#include "Crashed/HealthComponent.h"
#include "Engine/World.h"

AHitscanWeapon::AHitscanWeapon()
{
	bAutoFire = true;
	FireRate = 1.f;
}

void AHitscanWeapon::Fire(const FVector& Origin, const FVector& AimPoint)
{
	FVector ShootDir = (AimPoint - Origin);
	ShootDir.Z = 0.f;
	ShootDir = ShootDir.GetSafeNormal();

	if (ShootDir.IsNearlyZero()) return;

	const FVector End = Origin + ShootDir * Range;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		Origin,
		End,
		ECC_Visibility,
		Params
	);

	const FVector BeamEnd = bHit ? Hit.ImpactPoint : End;

	if (bHit)
	{
		if (AActor* HitActor = Hit.GetActor())
		{
			if (UHealthComponent* HealthComp = HitActor->FindComponentByClass<UHealthComponent>())
			{
				HealthComp->ApplyDamage(Damage);
			}
		}
	}
	
	WeaponFired.Broadcast(Origin, BeamEnd, bHit, Hit.Location);
}