#pragma once

#include "CoreMinimal.h"
#include "Crashed/NPC/BaseEnemy.h"
#include "ForestAnt.generated.h"

class AAntQueen;
class AAntHive;

UENUM(BlueprintType)
enum class EAntType : uint8
{
	Worker  UMETA(DisplayName = "Worker"),
	Soldier UMETA(DisplayName = "Soldier"),
	Healer  UMETA(DisplayName = "Healer")
};

UCLASS(Abstract)
class CRASHED_API AForestAnt : public ABaseEnemy
{
	GENERATED_BODY()

public:
	AForestAnt();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ant")
	EAntType AntType = EAntType::Worker;

	// Set by the queen after spawning — lets the AI controller redirect movement
	UPROPERTY(BlueprintReadOnly, Category = "Ant")
	TWeakObjectPtr<AAntHive> HomeHive = nullptr;
	
	void SetHomeHive(AAntHive* InHive);
	
	// Called by the queen after spawning
	void RegisterWithQueen(AAntQueen* InQueen);

protected:
	virtual void BeginPlay() override;
	virtual void OnDeath_Implementation() override;
	virtual void PerformAttack_Implementation() override;

private:
	TWeakObjectPtr<AAntQueen> Queen;
};