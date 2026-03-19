#include "ProjectileWeapon.h"
#include "WeaponProjectile.h"
#include "Engine/World.h"

void AProjectileWeapon::Fire(const FVector& Origin, const FVector& AimPoint)
{
	if (!ProjectileClass) return;

	FVector ShootDir = (AimPoint - Origin);
	ShootDir.Z = 0.f;
	ShootDir = ShootDir.GetSafeNormal();
	if (ShootDir.IsNearlyZero()) return;

	// Detonate at cursor OR at range, whichever is closer
	const float DistToCursor = FVector::Dist2D(Origin, AimPoint);

	float TravelDist;

	if (stopAtCursor)
	{
		TravelDist = FMath::Min(DistToCursor, Range);
	}
	else
	{
		TravelDist = FMath::Max(DistToCursor, Range);
	}
	

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();


	AWeaponProjectile* Projectile = GetWorld()->SpawnActor<AWeaponProjectile>(
		ProjectileClass, Origin, ShootDir.Rotation(), SpawnParams);

	if (Projectile)
	{
		Projectile->Launch(ShootDir, TravelDist);
	}
}
