#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

UCLASS()
class CRASHED_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float AISightRadius = 500.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float AISightAge = 5.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float AILoseSightRadius = AISightRadius + 50.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float AIFieldOfView = 90.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UAISenseConfig_Sight* SightConfig; 
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) 
	float DistanceToPlayer = 0.0f;

	
	AEnemyAIController();
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* MyPawn) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual FRotator GetControlRotation() const override;
	UFUNCTION()
	void OnPawnDetected(const TArray<AActor*>& DetectedPawns);

	
	UPROPERTY(transient)
	class UBehaviorTreeComponent* BTC;
	
	UPROPERTY(transient, BlueprintReadOnly, Category = "AI")
	UBlackboardComponent* BBC;
	
	uint16 EnemyKeyID;
	
protected:
	
};