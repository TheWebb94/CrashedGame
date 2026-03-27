#include "PlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Crashed/HealthComponent.h"
#include "Crashed/Weapons/BaseWeapon.h"



APlayerCharacter::APlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule/root does not rotate at all — we rotate the meshes manually
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 1200.0f;
    SpringArm->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
    SpringArm->bDoCollisionTest = false;
    SpringArm->bInheritPitch = false;
    SpringArm->bInheritYaw = false;
    SpringArm->bInheritRoll = false;

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    CameraComponent->SetupAttachment(SpringArm);

    // Both meshes attach to the capsule root, not to each other,
    // so the turret can rotate independently of the base
    BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
    BaseMesh->SetupAttachment(RootComponent);

    TurretMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TurretMesh"));
    TurretMesh->SetupAttachment(RootComponent);

    LastMoveDirection = FVector::ForwardVector;
    PlayerControllerRef = nullptr;
    
    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    PlayerControllerRef = Cast<APlayerController>(GetController());

    for (TSubclassOf<ABaseWeapon>& WeaponClass : WeaponClasses)
    {
        
        FVector SpawnLocation = FVector(0,0,-3000.0f); // spawns these weapons below map (im sure theres a better way to do this...)
        if (!WeaponClass) continue;
        FActorSpawnParameters Params;
        Params.Owner = this;
        Params.Instigator = this;
        ABaseWeapon* W = GetWorld()->SpawnActor<ABaseWeapon>(
            WeaponClass, SpawnLocation, FRotator::ZeroRotator, Params);
        if (W) SpawnedWeapons.Add(W);
    }

    if (SpawnedWeapons.Num() > 0)
        SwitchWeapon(0);
}

void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Track last movement direction from actual velocity
    FVector Velocity = GetVelocity();
    Velocity.Z = 0.f;
    if (!Velocity.IsNearlyZero())
    {
        LastMoveDirection = Velocity.GetSafeNormal();
    }

    RotateBase(DeltaTime);
    RotateTurret(DeltaTime);
}

void APlayerCharacter::StartFire()
{
    if (!EquippedWeapon) return;
    bIsFiring = true; 
    Fire();  

    if (EquippedWeapon->bAutoFire)
    {
        const float Interval = 1.f / FMath::Max(EquippedWeapon->FireRate, 0.01f);
        GetWorldTimerManager().SetTimer(
            FireTimerHandle, this, &APlayerCharacter::Fire, Interval, true);
    }
}

void APlayerCharacter::StopFire()
{
    bIsFiring = false;
    GetWorldTimerManager().ClearTimer(FireTimerHandle);
    FireTimerHandle.Invalidate(); //fixes bug where timer was invalidated upon switching weapons and clearing timerhandle
}

void APlayerCharacter::SwitchWeapon(int32 Index)
{
    if (!SpawnedWeapons.IsValidIndex(Index)) return;

    const bool bWasFiring = bIsFiring; 
    StopFire();

    CurrentWeaponIndex = Index;
    EquippedWeapon = SpawnedWeapons[Index];

    UStaticMesh* NewMesh = EquippedWeapon->GetWeaponStaticMesh();
    if (NewMesh)
    {
        TurretMesh->SetStaticMesh(NewMesh);
        TurretMesh->SetRelativeScale3D(EquippedWeapon->MeshScale);
    }
        
    if (bWasFiring)   
        StartFire();
}

void APlayerCharacter::RotateBase(float DeltaTime)
{
    const FRotator TargetRotation = LastMoveDirection.Rotation();
    const FRotator CurrentRotation = BaseMesh->GetComponentRotation();
    const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, BaseRotationSpeed);
    BaseMesh->SetWorldRotation(NewRotation);
}

void APlayerCharacter::RotateTurret(float DeltaTime)
{
    FVector AimPoint;
    if (!GetMouseAimPoint(AimPoint))
    {
        return;
    }

    const FVector AimDirection = (AimPoint - GetActorLocation()).GetSafeNormal2D();
    if (AimDirection.IsNearlyZero())
    {
        return;
    }

    FRotator TargetRotation = AimDirection.Rotation();
    if (EquippedWeapon)
    {
        TargetRotation.Yaw += EquippedWeapon->MeshYawOffset;
        TargetRotation.Roll += EquippedWeapon->MeshRollOffset;
    }
    const FRotator CurrentRotation = TurretMesh->GetComponentRotation();
    const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, TurretRotationSpeed);
    TurretMesh->SetWorldRotation(NewRotation);
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);

    PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &APlayerCharacter::StartSprint);
    PlayerInputComponent->BindAction("Sprint", IE_Released, this, &APlayerCharacter::StopSprint);

    PlayerInputComponent->BindAction("Fire", IE_Pressed,  this, &APlayerCharacter::StartFire);
    PlayerInputComponent->BindAction("Fire", IE_Released, this, &APlayerCharacter::StopFire);
    PlayerInputComponent->BindAction("NextWeapon",  IE_Pressed, this, &APlayerCharacter::NextWeapon);
    PlayerInputComponent->BindAction("PrevWeapon",  IE_Pressed, this, &APlayerCharacter::PrevWeapon);
    PlayerInputComponent->BindAction("Weapon1",     IE_Pressed, this, &APlayerCharacter::SelectWeapon1);
    PlayerInputComponent->BindAction("Weapon2",     IE_Pressed, this, &APlayerCharacter::SelectWeapon2);
    PlayerInputComponent->BindAction("Weapon3",     IE_Pressed, this, &APlayerCharacter::SelectWeapon3);
    PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &APlayerCharacter::Dash);
}

void APlayerCharacter::NextWeapon()   { SwitchWeapon((CurrentWeaponIndex + 1) % FMath::Max(SpawnedWeapons.Num(), 1)); }
void APlayerCharacter::PrevWeapon()   { SwitchWeapon((CurrentWeaponIndex - 1 + SpawnedWeapons.Num()) % FMath::Max(SpawnedWeapons.Num(), 1)); }
void APlayerCharacter::SelectWeapon1(){ SwitchWeapon(0); }
void APlayerCharacter::SelectWeapon2(){ SwitchWeapon(1); }
void APlayerCharacter::SelectWeapon3(){ SwitchWeapon(2); }

void APlayerCharacter::MoveForward(float Value)
{
    if (Value != 0.f)
    {
        AddMovementInput(FVector::ForwardVector, Value);
    }
}

void APlayerCharacter::MoveRight(float Value)
{
    if (Value != 0.f)
    {
        AddMovementInput(FVector::RightVector, Value);
    }
}

bool APlayerCharacter::GetMouseAimPoint(FVector& OutAimPoint) const
{
    if (!PlayerControllerRef)
    {
        return false;
    }

    FHitResult HitResult;
    const bool bHit = PlayerControllerRef->GetHitResultUnderCursorByChannel(
        UEngineTypes::ConvertToTraceType(ECC_Visibility),
        true,
        HitResult
    );

    if (bHit)
    {
        OutAimPoint = HitResult.ImpactPoint;
        return true;
    }

    return false;
}

void APlayerCharacter::StartSprint()
{
    GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void APlayerCharacter::StopSprint()
{
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void APlayerCharacter::Fire()
{
    if (!EquippedWeapon) return;
    
    const float MinInterval = 1.f / FMath::Max(EquippedWeapon->FireRate, 0.01f);
    const float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastFireTime < MinInterval) return;
    LastFireTime = Now;

    FVector AimPoint;
    if (!GetMouseAimPoint(AimPoint)) return;

    EquippedWeapon->Fire(GetActorLocation(), AimPoint);
}

void APlayerCharacter::Dash()
{
}
