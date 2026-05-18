#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "BaseEnemy.generated.h"


class UHealthComponent;
class ABaseWeapon;
class UBehaviorTree;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyDeath, ABaseEnemy*, DeadEnemy);

UCLASS(Abstract)
class CRASHED_API ABaseEnemy : public ACharacter
{
    GENERATED_BODY()

public:
    ABaseEnemy();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    
    UBehaviorTree* GetBehaviorTree() const { return BehaviorTreeAsset; }

    //components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UHealthComponent* HealthComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* BaseMesh;

    //stats 
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Stats")
    float MoveSpeed = 300.f;

    //distance at which the AIController starts tracking the player
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Stats")
    float DetectionRadius = 1500.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Stats")
    float AttackRange = 150.f;

    //used when dealing damage directly without a weapon actor
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Stats")
    float AttackDamage = 20.f;

    //attacks per second
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Stats")
    float AttackRate = 1.f;

    //TODO: this is not in use, cleanup or implement units that use the weapon system
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Combat")
    TSubclassOf<ABaseWeapon> WeaponClass;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
    ABaseWeapon* EquippedWeapon = nullptr;

    //behaviour tree assigned this in the child blueprint — EnemyAIController runs it on possess
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
    UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    class APatrolPath* PatrolPath;
    
    //delegate - callable in BP
    UPROPERTY(BlueprintAssignable, Category = "Enemy|Events")
    FOnEnemyDeath OnEnemyDeath;

    UFUNCTION(BlueprintCallable, Category = "Enemy")
    UHealthComponent* GetHealthComponent() const { return HealthComponent; }

    //called from BT Tasks to trigger this enemy's attack.
    //C++ children override PerformAttack_Implementation().
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Enemy|Combat")
    void PerformAttack();
    virtual void PerformAttack_Implementation();
    
    UPROPERTY()
    AActor* CurrentAttackTarget = nullptr;

protected:
    //bound to HealthComponent->OnDeath in BeginPlay.
    //override to play a death animation or drop loot before destroying.
    UFUNCTION(BlueprintNativeEvent, Category = "Enemy|Events")
    void OnDeath();
    
    UFUNCTION()
    virtual void OnDeath_Implementation();

    //rotates BaseMesh to face the movement direction each tick
    virtual void RotateBase(float DeltaTime);

    UPROPERTY(EditDefaultsOnly, Category = "Enemy|Movement")
    float BaseRotationSpeed = 8.f; //rate of rotation towards movement vector

private:
    FVector LastMoveDirection;
};
