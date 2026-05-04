// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SpiderAIController.generated.h"

UCLASS()
class CRASHED_API ASpiderAIController : public AAIController
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASpiderAIController();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float AISightRadius = 1000.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float AISightAge = 5.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float AILoseSightRadius = AISightRadius + 50.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float AIFieldOfView = 130.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UAISenseConfig_Sight* SightConfig;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bIsPlayerDetected = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float DistanceToOther = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float AttackRange = 200.0f;


	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* MyPawn) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual FRotator GetControlRotation() const override;
	UFUNCTION()
	void OnPawnDetected(const TArray<AActor*>& DetectedPawns);
	
};
