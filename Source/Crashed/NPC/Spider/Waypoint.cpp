// Fill out your copyright notice in the Description page of Project Settings.


#include "Waypoint.h"

#include "Spider.h"
#include "Crashed/NPC/BaseEnemy.h"
#include "Crashed/NPC/EnemyAIController.h"


// Sets default values
AWaypoint::AWaypoint()
{
	PrimaryActorTick.bCanEverTick = true;
	
	//setup root component
	Root = CreateDefaultSubobject <USceneComponent>("Root");
	SetRootComponent(Root);
	
	//Add box component
	Box = CreateDefaultSubobject<UBoxComponent>("Box");
	Box->SetupAttachment(GetRootComponent());
	Box->OnComponentBeginOverlap.AddDynamic(this, &AWaypoint::OnCharacterEnter); //bind event to it
}

//When character enters 
void AWaypoint::OnCharacterEnter(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ASpider* Spider = nullptr;
	
	if (OtherActor != nullptr) //safety guard
	{
		Spider = Cast<ASpider>(OtherActor);
		if (Spider != nullptr) //guard
		{
			Spider->NextWayPoint = NextWayPoint; //sets the next waypoint
		}
	}
}

// Called when the game starts or when spawned
void AWaypoint::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWaypoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

