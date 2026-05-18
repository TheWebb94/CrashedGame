#include "BaseEnemy.h"
#include "EnemyAIController.h"

#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Crashed/HealthComponent.h"
#include "Crashed/Weapons/BaseWeapon.h"
#include "Engine/World.h"

ABaseEnemy::ABaseEnemy()
{
    PrimaryActorTick.bCanEverTick = true;

    // Mesh rotation is driven manually in RotateBase
    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
    GetCharacterMovement()->bUseRVOAvoidance = true; // fixes traffic issue of ants getting stuck when walking the same path the wrong way in tight corridoors
    GetCharacterMovement()->AvoidanceConsiderationRadius = 80.f;

    //setup mesh component in obj
    BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
    BaseMesh->SetupAttachment(RootComponent);

    //modular health component fopr all characters
    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

    //AI controller assignment
    AIControllerClass = AEnemyAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    //store recent move vector (to prevent erratic reorientation)
    LastMoveDirection = FVector::ForwardVector;
}

void ABaseEnemy::BeginPlay()
{
    Super::BeginPlay();

    //configure base values
    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
    BaseMesh->SetMobility(EComponentMobility::Movable);


    if (HealthComponent)
    {
        HealthComponent->OnDeath.AddDynamic(this, &ABaseEnemy::OnDeath_Implementation); //bind a death event  to the health component notifier
    }

    if (WeaponClass) //weapon class not in use for NPCs, only player
    {
        FActorSpawnParameters Params;
        Params.Owner = this;
        Params.Instigator = this;
        EquippedWeapon = GetWorld()->SpawnActor<ABaseWeapon>(
            WeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);
    }
}

// Tick — only handles mesh rotation, AI is driven by the Behavior Tree
void ABaseEnemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FVector Velocity = GetVelocity();
    Velocity.Z = 0.f;
    if (!Velocity.IsNearlyZero()) //if not moving, or near zero movement, maintain last strong movement vector
    {
        LastMoveDirection = Velocity.GetSafeNormal();
    }

    RotateBase(DeltaTime); //ensure mesh is rotated towards the stored lastmovedir
}

void ABaseEnemy::PerformAttack_Implementation()
{
    //child classes implemeent if requireed
}

//on death broadcast the death and cleanup actor
void ABaseEnemy::OnDeath_Implementation()
{
    OnEnemyDeath.Broadcast(this);
    Destroy();
}

//Apply the rotation to the mesh, matches movement vector with guards against low values. rotates ate rotation rate specified in header
void ABaseEnemy::RotateBase(float DeltaTime)
{
    FVector TargetDir = LastMoveDirection;

    if (CurrentAttackTarget && GetVelocity().IsNearlyZero())
    {
        FVector ToTarget = CurrentAttackTarget->GetActorLocation() - GetActorLocation();
        ToTarget.Z = 0.f;
        if (!ToTarget.IsNearlyZero())
            TargetDir = ToTarget.GetSafeNormal();
    }

    const FRotator TargetRotation  = TargetDir.Rotation();
    const FRotator CurrentRotation = BaseMesh->GetComponentRotation();
    const FRotator NewRotation     = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, BaseRotationSpeed);
    BaseMesh->SetWorldRotation(NewRotation);
}

