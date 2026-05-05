#include "Spider.h"

#include "SpiderWebProjectile.h"
#include "DefensiveWeb.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

ASpider::ASpider()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASpider::BeginPlay()
{
	Super::BeginPlay();
}

void ASpider::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASpider::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ASpider::ShootWebProjectile(AActor* Target)
{
	if (!WebProjectileClass || !Target || !bCanRangedAttack) return;

	FVector SpawnLoc = GetActorLocation();
	FVector Direction = (Target->GetActorLocation() - SpawnLoc).GetSafeNormal();
	FRotator SpawnRot = Direction.Rotation();

	FActorSpawnParameters Params;
	Params.Owner   = this;
	Params.Instigator = GetInstigator();

	ASpiderWebProjectile* Projectile = GetWorld()->SpawnActor<ASpiderWebProjectile>(
		WebProjectileClass, SpawnLoc, SpawnRot, Params);

	if (Projectile)
	{
		// Velocity is set by ProjectileMovementComponent using its InitialSpeed + forward direction
		// (SpawnRot already aligns the component's forward to Direction)
		bCanRangedAttack = false;
		GetWorldTimerManager().SetTimer(RangedCooldownTimer, [this]()
		{
			bCanRangedAttack = true;
		}, RangedAttackCooldown, false);
	}
}

void ASpider::PlaceDefensiveWeb()
{
	if (!DefensiveWebClass) return;

	// Place flat on the ground beneath the spider
	FVector SpawnLoc = GetActorLocation() - FVector(0.f, 0.f, GetCapsuleComponent()
		? GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 50.f);

	FActorSpawnParameters Params;
	Params.Owner = this;

	GetWorld()->SpawnActor<ADefensiveWeb>(DefensiveWebClass, SpawnLoc, FRotator::ZeroRotator, Params);
}