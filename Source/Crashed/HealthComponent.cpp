// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	currentHealth = maxHealth;
	// ...
	
}


// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UHealthComponent::ApplyDamage(float amount)
{
	if (IsDead()) return;
	
	currentHealth = FMath::Clamp(currentHealth - amount, 0, maxHealth);
	OnHealthChanged.Broadcast(currentHealth, maxHealth);
	
	if (currentHealth <= 0)
	{
		AActor* DamagedActor = GetOwner();
		
		if (DamagedActor)
		{
			OnDeath.Broadcast();
		}
	}
}

void UHealthComponent::ApplyHealth(float amount)
{
	currentHealth = FMath::Clamp(currentHealth + amount, 0, maxHealth);
	OnHealthChanged.Broadcast(currentHealth, maxHealth);
}

float UHealthComponent::GetCurrentHealth() const
{
	return currentHealth;
}

float UHealthComponent::GetMaxHealth() const
{
	return maxHealth;
}

float UHealthComponent::GetHealthPercent() const
{
	return currentHealth / maxHealth;
}

bool UHealthComponent::IsDead()
{
	if (currentHealth <= 0)
	{
		return true;
	}
	return false;
	
}

