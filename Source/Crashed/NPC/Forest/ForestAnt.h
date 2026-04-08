#pragma once

#include "CoreMinimal.h"
#include "Crashed/NPC/BaseEnemy.h"
#include "ForestAnt.generated.h"

class AAntQueen;

UENUM(BlueprintType)
enum class EAntType : uint8
{
	Worker  UMETA(DisplayName = "Worker"),
	Soldier UMETA(DisplayName = "Soldier")
};

UCLASS(Abstract)
class CRASHED_API AForestAnt : public ABaseEnemy
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ant")
	EAntType AntType = EAntType::Worker;

	// Called by the queen after spawning this ant
	void RegisterWithQueen(AAntQueen* InQueen);

protected:
	virtual void OnDeath_Implementation() override;

private:
	TWeakObjectPtr<AAntQueen> Queen;
};
