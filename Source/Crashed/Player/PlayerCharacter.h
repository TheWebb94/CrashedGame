#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Crashed/Weapons/BaseWeapon.h"
#include "PlayerCharacter.generated.h"


class ABaseWeapon;


UCLASS()
class CRASHED_API APlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    APlayerCharacter();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    class UHealthComponent* HealthComponent;
    
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    void MoveForward(float Value);
    void MoveRight(float Value);
    bool GetMouseAimPoint(FVector& OutAimPoint) const;

    void StartSprint();
    void StopSprint();

    void Fire();
    void Dash();

protected:
    //Components
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class USpringArmComponent* SpringArm;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UCameraComponent* CameraComponent;

    // Base  mesh 
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* BaseMesh;

    // Turret mesh 
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* TurretMesh;

    //References
    APlayerController* PlayerControllerRef;

    //Movement
    UPROPERTY(EditAnywhere, Category = "Movement")
    float WalkSpeed = 400.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float SprintSpeed = 800.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float BaseRotationSpeed = 8.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float TurretRotationSpeed = 15.f;

    FVector LastMoveDirection;
    
    //TODO: implement an interact behaviour
    void Interact();

    //Combat
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
    ABaseWeapon* EquippedWeapon = nullptr;
    
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    TArray<TSubclassOf<ABaseWeapon>> WeaponClasses;  // fill in BP with MachineGun, Rocket, Gauss BPs

    UPROPERTY()
    TArray<ABaseWeapon*> SpawnedWeapons;

    int32 CurrentWeaponIndex = 0;

    void StartFire();
    void StopFire();
    void NextWeapon();
    void PrevWeapon();
    void SelectWeapon1();
    void SelectWeapon2();
    void SelectWeapon3();
    void SwitchWeapon(int32 Index);

private:
    void RotateBase(float DeltaTime);
    void RotateTurret(float DeltaTime);
    bool bIsFiring = false;
    FTimerHandle FireTimerHandle;
    float LastFireTime = -1.f;
};
