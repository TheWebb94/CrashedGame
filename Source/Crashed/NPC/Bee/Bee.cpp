// Fill out your copyright notice in the Description page of Project Settings.


#include "Bee.h"


// Sets default values
ABee::ABee()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ABee::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABee::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ABee::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

