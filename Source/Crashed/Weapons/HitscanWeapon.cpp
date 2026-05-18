#include "HitscanWeapon.h"
#include "DrawDebugHelpers.h"
#include "Crashed/HealthComponent.h"
#include "Engine/World.h"

AHitscanWeapon::AHitscanWeapon()
{
	bAutoFire = true;
	FireRate = 1.f;
}

//fire weeapon implementation
void AHitscanWeapon::Fire(const FVector& Origin, const FVector& AimPoint)
{
	FVector ShootDir = (AimPoint - Origin); //get shoot direction from aim point param - origin(playerloc)
	ShootDir.Z = 0.f;
	ShootDir = ShootDir.GetSafeNormal();

	if (ShootDir.IsNearlyZero()) return; //if direction is not valid, return

	const FVector End = Origin + ShootDir * Range; //shoot in calculated direction, with amx range implemented

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this); // actors to not interact with (self)
	Params.AddIgnoredActor(GetOwner());

	//raytrace params
	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		Origin,
		End,
		ECC_Visibility,
		Params
	);

	const FVector BeamEnd = bHit ? Hit.ImpactPoint : End; //determine hit result

	if (bHit) //on hit access health component and deal damage
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