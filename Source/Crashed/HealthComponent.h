// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CRASHED_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	float maxHealth = 100.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	float currentHealth = 100;
	
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnDeath OnDeath;

	UFUNCTION(BlueprintCallable, Category = "Health")
	void ApplyDamage(float amount);
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	void ApplyHealth(float amount);
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetCurrentHealth() const; 
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetMaxHealth() const;
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetHealthPercent() const;
	
	bool IsDead(); 
	
private:
	
	
};
