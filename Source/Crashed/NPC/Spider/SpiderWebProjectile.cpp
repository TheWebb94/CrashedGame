#include "SpiderWebProjectile.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "Crashed/NPC/Bee/Bee.h"
#include "Crashed/NPC/Bee/BeeAIController.h"
#include "Crashed/NPC/Bee/BeeHive.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

ASpiderWebProjectile::ASpiderWebProjectile()
{
    PrimaryActorTick.bCanEverTick = false;

    //config for collision sphere
    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    CollisionSphere->InitSphereRadius(20.f);
    CollisionSphere->SetCollisionProfileName(TEXT("Projectile"));
    CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    CollisionSphere->SetGenerateOverlapEvents(true);

    RootComponent = CollisionSphere;

    //config for projectile mesh
    WebMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WebMesh"));
    WebMesh->SetupAttachment(CollisionSphere);
    WebMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    //config for projectile movement
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->InitialSpeed = ProjectileSpeed;
    ProjectileMovement->MaxSpeed    = ProjectileSpeed;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->ProjectileGravityScale   = 0.2f;

    //destroys projectile automatically if it does not hit anything
    InitialLifeSpan = 5.f;
}

void ASpiderWebProjectile::BeginPlay()
{
    Super::BeginPlay();

    //binds overlap event when projectile enters play
    CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ASpiderWebProjectile::OnOverlap);
}

void ASpiderWebProjectile::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    //guard against invalid actors and prevents hitting the owner
    if (!OtherActor || OtherActor == GetOwner()) return;

    //aggro whole swarm before entangling so the hive timer starts correctly
    if (ABee* HitBee = Cast<ABee>(OtherActor))
    {
        //sends hive bees to attack the projectile owner
        if (HitBee->OwnerHive.IsValid())
            HitBee->OwnerHive->SendBeesToAttack(GetOwner());
    }

    //entangles the hit actor and destroys the projectile
    EntangleTarget(OtherActor);
    Destroy();
}

void ASpiderWebProjectile::EntangleTarget(AActor* Target)
{
    //prevents self entangle
    if (Target == GetOwner()) return;
    
    //only characters can be entangled
    ACharacter* Char = Cast<ACharacter>(Target);
    if (!Char) return;

    //gets movement component and prevents re-entangling already stopped targets
    UCharacterMovementComponent* MoveComp = Char->GetCharacterMovement();
    if (!MoveComp || MoveComp->MaxWalkSpeed <= 0.f) return;

    //stores original movement speed before immobilising target
    float OriginalSpeed = MoveComp->MaxWalkSpeed;
    MoveComp->MaxWalkSpeed = 0.f;

    //uses weak pointer so the timer does not access destroyed targets
    TWeakObjectPtr<UCharacterMovementComponent> WeakMove(MoveComp);

    //restores movement speed after entangle duration
    FTimerHandle Handle;
    GetWorldTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([WeakMove, OriginalSpeed]()
    {
        //only restores speed if movement component still exists
        if (WeakMove.IsValid())
            WeakMove->MaxWalkSpeed = OriginalSpeed;
    }), EntangleDuration, false);
}