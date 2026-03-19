#include "WeaponProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Crashed/HealthComponent.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Crashed/Player/PlayerCharacter.h"



AWeaponProjectile::AWeaponProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComp->InitSphereRadius(16.f);
	CollisionComp->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	RootComponent = CollisionComp;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	ProjectileMesh->SetRelativeScale3D(FVector(MeshScale));
	
	CollisionComp->SetNotifyRigidBodyCollision(true);
}

void AWeaponProjectile::BeginPlay()
{
	Super::BeginPlay();
	ProjectileMesh->SetRelativeScale3D(FVector(MeshScale));
	CollisionComp->OnComponentHit.AddDynamic(this, &AWeaponProjectile::OnHit);

	if (AActor* OwnerActor = GetOwner())
	{
		CollisionComp->IgnoreActorWhenMoving(OwnerActor, true);
	}
	if (APawn* InstigatorPawn = GetInstigator())
	{
		CollisionComp->IgnoreActorWhenMoving(InstigatorPawn, true);
		InstigatorPawn->MoveIgnoreActorAdd(this);
	}
}


void AWeaponProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!bLaunched) return;
	if (!OtherActor || OtherActor == GetOwner() || OtherActor == GetInstigator()) return;

	UHealthComponent* HealthComp = OtherActor->FindComponentByClass<UHealthComponent>();
	if (HealthComp)
	{
		HealthComp->ApplyDamage(Damage);
	}

	Destroy();
}


void AWeaponProjectile::Launch(const FVector& InDirection, float InTravelDistance)
{
	Direction = InDirection.GetSafeNormal();
	TravelDistance = InTravelDistance;
	DistanceTraveled = 0.f;
	bLaunched = true;
}

void AWeaponProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bLaunched) return;

	const float StepDistance = Speed * DeltaTime;
	SetActorLocation(GetActorLocation() + Direction * StepDistance, true);
	DistanceTraveled += StepDistance;

	if (DistanceTraveled >= TravelDistance)
	{
		Explode();
	}
}

void AWeaponProjectile::Explode()
{
	bLaunched = false;

	const FVector Center = GetActorLocation();

	DrawDebugSphere(GetWorld(), Center, ExplosionRadius, 16, FColor::Orange, false, 2.f, 0, 3.f);

	TArray<AActor*> OverlappedActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	//ActorsToIgnore.Add(Player);

	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(), Center, ExplosionRadius, ObjectTypes, nullptr, ActorsToIgnore, OverlappedActors);

	for (AActor* Actor : OverlappedActors)
	{
		UHealthComponent* targetHealthComponent = Actor->FindComponentByClass<UHealthComponent>();
		if (targetHealthComponent)
		{
			targetHealthComponent->ApplyDamage(Damage);
		}
	}

	Destroy();
}
