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

    //setup the spring arm for the camera attachment
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 1200.0f;
    SpringArm->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
    SpringArm->bDoCollisionTest = false;
    SpringArm->bInheritPitch = false;
    SpringArm->bInheritYaw = false;
    SpringArm->bInheritRoll = false;

    //add the camera to the spring arm
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
    
    //setup health component attachment
    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    PlayerControllerRef = Cast<APlayerController>(GetController());

    for (TSubclassOf<ABaseWeapon>& WeaponClass : WeaponClasses)
    {
        
        FVector SpawnLocation = FVector(0,0,-3000.0f); // spawns these weapons below map (im sure theres a better way to do this...)
        if (!WeaponClass) continue;                                 // weapons were not working without them being spawned in the level?
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

    //rotate base towards movement vector, rotate turret towards vector(dire frokm player to mouse location)
    RotateBase(DeltaTime);
    RotateTurret(DeltaTime);
}

//for firing of equipped weapon
void APlayerCharacter::StartFire()
{
    if (!EquippedWeapon) return; //safety guard
    bIsFiring = true; 
    Fire();  //trigger equipped weapon specific fire behaviour

    if (EquippedWeapon->bAutoFire) //if full auto, refire at fire rate until mouse unclicked
    {
        const float Interval = 1.f / FMath::Max(EquippedWeapon->FireRate, 0.01f);
        GetWorldTimerManager().SetTimer(
            FireTimerHandle, this, &APlayerCharacter::Fire, Interval, true);
    }
}

//safe management of stopping the firing 
void APlayerCharacter::StopFire()
{
    bIsFiring = false;
    GetWorldTimerManager().ClearTimer(FireTimerHandle);
    FireTimerHandle.Invalidate(); //fixes bug where timer was invalidated upon switching weapons and clearing timerhandle
}

//returns index of weapon list to set the equipped weapon to - called with mousewheel or numkeys
void APlayerCharacter::SwitchWeapon(int32 Index)
{
    if (!SpawnedWeapons.IsValidIndex(Index)) return;

    const bool bWasFiring = bIsFiring; //cancels any active firing
    StopFire();

    CurrentWeaponIndex = Index;
    EquippedWeapon = SpawnedWeapons[Index]; //sets weapon
    
    OnWeaponChanged.Broadcast(EquippedWeapon); //event dispatcher so HUD can bind to this function
    
    UStaticMesh* NewMesh = EquippedWeapon->GetWeaponStaticMesh(); //applies mesh change to player
    if (NewMesh)
    {
        TurretMesh->SetStaticMesh(NewMesh);
        TurretMesh->SetRelativeScale3D(EquippedWeapon->MeshScale);
    }
        
    if (bWasFiring)   
        StartFire(); //if was firing, restart firing now new weapon is attached
}

//rotate base towards movement dir
void APlayerCharacter::RotateBase(float DeltaTime)
{
    const FRotator TargetRotation = LastMoveDirection.Rotation();
    const FRotator CurrentRotation = BaseMesh->GetComponentRotation();
    const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, BaseRotationSpeed);
    BaseMesh->SetWorldRotation(NewRotation);
}

//gets the location of the mouse, and makes turret rotate to face that direction
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

///<summary>
///sets up the input actions so they can be controlled centrally
///</summary>
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
}

void APlayerCharacter::NextWeapon()   { SwitchWeapon((CurrentWeaponIndex + 1) % FMath::Max(SpawnedWeapons.Num(), 1)); }
void APlayerCharacter::PrevWeapon()   { SwitchWeapon((CurrentWeaponIndex - 1 + SpawnedWeapons.Num()) % FMath::Max(SpawnedWeapons.Num(), 1)); }
void APlayerCharacter::SelectWeapon1(){ SwitchWeapon(0); }
void APlayerCharacter::SelectWeapon2(){ SwitchWeapon(1); }
void APlayerCharacter::SelectWeapon3(){ SwitchWeapon(2); }

//move forwards/backwards implementaiton
void APlayerCharacter::MoveForward(float Value)
{
    if (Value != 0.f)
    {
        AddMovementInput(FVector::ForwardVector, Value);
    }
}

//move left/right implementaiton
void APlayerCharacter::MoveRight(float Value)
{
    if (Value != 0.f)
    {
        AddMovementInput(FVector::RightVector, Value);
    }
}


bool APlayerCharacter::GetMouseAimPoint(FVector& OutAimPoint) const
{
    if (!PlayerControllerRef) //safety guard
    {
        return false;
    }

    FHitResult HitResult;
    //get mouse location
    const bool bHit = PlayerControllerRef->GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility),true,HitResult);

    if (bHit)
    {
        OutAimPoint = HitResult.ImpactPoint; //collect aim point
        return true;
    }

    return false;
}

//sprint implementation
void APlayerCharacter::StartSprint()
{
    GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

//on sprint eend implementation
void APlayerCharacter::StopSprint()
{
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

//Fire methoid, has different fuinctioanilty depending on the equipped weapon
void APlayerCharacter::Fire()
{
    if (!EquippedWeapon) return; //safety guard
    
    const float MinInterval = 1.f / FMath::Max(EquippedWeapon->FireRate, 0.01f); //check fire rate
    const float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastFireTime < MinInterval) return; //if fired too recently cant fire again
    LastFireTime = Now; //otherwise reset timer

    FVector AimPoint;
    if (!GetMouseAimPoint(AimPoint)) return;

    EquippedWeapon->Fire(GetActorLocation(), AimPoint); //then fire towards cursor location
}

