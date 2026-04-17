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
    GetCharacterMovement()->bUseRVOAvoidance = true; // fixes issuee of ants getting stuck when walking the same path the wrong way in tight corridoors
    GetCharacterMovement()->AvoidanceConsiderationRadius = 50.f;

    BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
    BaseMesh->SetupAttachment(RootComponent);

    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

    AIControllerClass = AEnemyAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    LastMoveDirection = FVector::ForwardVector;
}

void ABaseEnemy::BeginPlay()
{
    Super::BeginPlay();

    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
    BaseMesh->SetMobility(EComponentMobility::Movable);


    if (HealthComponent)
    {
        HealthComponent->OnDeath.AddDynamic(this, &ABaseEnemy::OnDeath_Implementation);
    }

    if (WeaponClass)
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
    if (!Velocity.IsNearlyZero())
    {
        LastMoveDirection = Velocity.GetSafeNormal();
    }

    RotateBase(DeltaTime);
}

void ABaseEnemy::PerformAttack_Implementation()
{
    //child classes ikmplemeent if necessary
}

void ABaseEnemy::OnDeath_Implementation()
{
    OnEnemyDeath.Broadcast(this);
    Destroy();
}

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

