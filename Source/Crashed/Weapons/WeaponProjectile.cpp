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

	//setup sphere component on obj
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComp->InitSphereRadius(16.f);
	CollisionComp->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	RootComponent = CollisionComp;

	//setup mesh on obj
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMesh->SetRelativeScale3D(FVector(MeshScale));
	CollisionComp->SetNotifyRigidBodyCollision(true);
}

void AWeaponProjectile::BeginPlay()
{
	Super::BeginPlay();
	//ensure scale is set correctly
	ProjectileMesh->SetRelativeScale3D(FVector(MeshScale));
	CollisionComp->OnComponentHit.AddDynamic(this, &AWeaponProjectile::OnHit); //bind hit event to component overlap

	//ignore self/owner collision when firing projecile
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

//event bound to overlapped actors on projectile
void AWeaponProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//safety guards
	if (!bLaunched) return; 
	if (!OtherActor || OtherActor == GetOwner() || OtherActor == GetInstigator()) return;

	//access health component
	UHealthComponent* HealthComp = OtherActor->FindComponentByClass<UHealthComponent>();
	if (HealthComp)
	{
		APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
		APawn* ControlledPawn = PlayerController->GetPawn();
		HealthComp->ApplyDamageFrom(Damage, ControlledPawn); //apply damage with instigator info
	}

	Destroy(); //destroy projectile once target hit
}

//reconfigure variables for fired projectile, allows safety checks
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

	const FVector Start       = GetActorLocation();
	const float   StepDistance = Speed * DeltaTime;
	const FVector End          = Start + Direction * StepDistance;

	//explicit local sweep to ensure any valid targets are hit
	TArray<AActor*> OverlappedActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	TArray<AActor*> ActorsToIgnore = { this };
	if (GetOwner())     ActorsToIgnore.Add(GetOwner());		//ignoring self
	if (GetInstigator()) ActorsToIgnore.Add(GetInstigator());	//and owner

	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(), Start, CollisionComp->GetScaledSphereRadius(),
		ObjectTypes, nullptr, ActorsToIgnore, OverlappedActors);

	for (AActor* Actor : OverlappedActors) //if hitting a valid actor, apply damage correctly
	{
		UHealthComponent* HC = Actor->FindComponentByClass<UHealthComponent>();
		if (HC)
		{
			APlayerController* PC = GetWorld()->GetFirstPlayerController();
			HC->ApplyDamageFrom(Damage, PC ? PC->GetPawn() : nullptr);
			
		}
		Destroy();
		return;
	}

	SetActorLocation(End, true);
	DistanceTraveled += StepDistance;

	if (DistanceTraveled >= TravelDistance) //explode when reaching maximum range
		Explode();
}

void AWeaponProjectile::Explode()
{
	bLaunched = false;

	const FVector Center = GetActorLocation();
	
	//for calling functionality in blueprint
	OnExplode.Broadcast();
	
	//DrawDebugSphere(GetWorld(), Center, ExplosionRadius, 16, FColor::Orange, false, 2.f, 0, 3.f);

	TArray<AActor*> OverlappedActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	//ActorsToIgnore.Add(Player);
	
	//Gets a list of actors that overlap with explosion raidus
	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(), Center, ExplosionRadius, ObjectTypes, nullptr, ActorsToIgnore, OverlappedActors);

	for (AActor* Actor : OverlappedActors) //get all overlapping actors
	{
		UHealthComponent* targetHealthComponent = Actor->FindComponentByClass<UHealthComponent>(); //access health component
		if (targetHealthComponent)	//safety check
		{
			APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
			APawn* ControlledPawn = PlayerController->GetPawn();
			
			targetHealthComponent->ApplyDamageFrom(Damage, ControlledPawn); //apply instigated damage
		}
	}

	Destroy();
}
