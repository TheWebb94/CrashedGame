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

    // Mesh rotation is driven manually in RotateBase — same pattern as APlayerCharacter
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;

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
    // Base no-op. Child classes should either:
    //   - Call EquippedWeapon->Fire(GetActorLocation(), <target location>)
    //   - Or get the player and call HealthComponent->ApplyDamage(AttackDamage) directly
}

void ABaseEnemy::OnDeath_Implementation()
{
    OnEnemyDeath.Broadcast(this);
    Destroy();
}

void ABaseEnemy::RotateBase(float DeltaTime)
{
    const FRotator TargetRotation  = LastMoveDirection.Rotation();
    const FRotator CurrentRotation = BaseMesh->GetComponentRotation();
    const FRotator NewRotation     = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, BaseRotationSpeed);
    BaseMesh->SetWorldRotation(NewRotation);
}
