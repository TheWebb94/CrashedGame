#pragma once

#include "CoreMinimal.h"
#include "Crashed/NPC/BaseEnemy.h"
#include "Spider.generated.h"

class AWaypoint;
class ASpiderWebProjectile;
class ADefensiveWeb;

UCLASS()
class CRASHED_API ASpider : public ABaseEnemy
{
	GENERATED_BODY()

public:
	ASpider();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Patrol")
	AWaypoint* NextWayPoint;

	// Blueprint should assign these to the matching child classes
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat")
	TSubclassOf<ASpiderWebProjectile> WebProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat")
	TSubclassOf<ADefensiveWeb> DefensiveWebClass;

	// Seconds between ranged web shots
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat")
	float RangedAttackCooldown = 3.0f;

	bool bCanRangedAttack = true;

	// Fire a web projectile aimed at Target
	void ShootWebProjectile(AActor* Target);

	// Drop a defensive web trap at the spider's current feet position
	void PlaceDefensiveWeb();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	FTimerHandle RangedCooldownTimer;
};