#include "SpiderWebProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

ASpiderWebProjectile::ASpiderWebProjectile()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    CollisionSphere->InitSphereRadius(20.f);
    CollisionSphere->SetCollisionProfileName(TEXT("Projectile"));
    CollisionSphere->SetNotifyRigidBodyCollision(true);
    RootComponent = CollisionSphere;

    WebMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WebMesh"));
    WebMesh->SetupAttachment(CollisionSphere);
    WebMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->InitialSpeed = ProjectileSpeed;
    ProjectileMovement->MaxSpeed    = ProjectileSpeed;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->ProjectileGravityScale   = 0.2f;

    InitialLifeSpan = 5.f;
}

void ASpiderWebProjectile::BeginPlay()
{
    Super::BeginPlay();
    CollisionSphere->OnComponentHit.AddDynamic(this, &ASpiderWebProjectile::OnHit);
}

void ASpiderWebProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
    if (OtherActor && OtherActor != GetOwner())
    {
        EntangleTarget(OtherActor);
        Destroy();
    }
}

void ASpiderWebProjectile::EntangleTarget(AActor* Target)
{
    ACharacter* Char = Cast<ACharacter>(Target);
    if (!Char) return;

    UCharacterMovementComponent* MoveComp = Char->GetCharacterMovement();
    if (!MoveComp || MoveComp->MaxWalkSpeed <= 0.f) return;

    float OriginalSpeed = MoveComp->MaxWalkSpeed;
    MoveComp->MaxWalkSpeed = 0.f;

    TWeakObjectPtr<UCharacterMovementComponent> WeakMove(MoveComp);
    FTimerHandle Handle;
    GetWorldTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([WeakMove, OriginalSpeed]()
    {
        if (WeakMove.IsValid())
            WeakMove->MaxWalkSpeed = OriginalSpeed;
    }), EntangleDuration, false);
}