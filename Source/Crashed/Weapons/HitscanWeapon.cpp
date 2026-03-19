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

	// End point is always capped at Range — cursor distance doesn't matter
	const FVector End = Origin + ShootDir * Range;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());

	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Origin, End, ECC_Visibility, Params);

	if (bHit)
	{
		if (AActor* HitActor = Hit.GetActor())
		{
			UHealthComponent* HealthComp = HitActor->FindComponentByClass<UHealthComponent>();
			if (HealthComp)
			{
				HealthComp->ApplyDamage(Damage);
			}
		}
		DrawDebugLine(GetWorld(), Origin, Hit.ImpactPoint, FColor::Cyan, false, 1.f, 0, 2.f);
		DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 8.f, 8, FColor::White, false, 1.f);
	}
	else
	{
		DrawDebugLine(GetWorld(), Origin, End, FColor::Cyan, false, 1.f, 0, 2.f);
	}
}
