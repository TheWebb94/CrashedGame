// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Crashed/NPC/EnemyAIController.h"
#include "BeeAIController.generated.h"

UCLASS()
class CRASHED_API ABeeAIController : public AEnemyAIController
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABeeAIController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
