// Fill out your copyright notice in the Description page of Project Settings.


#include "Waypoint.h"

#include "Spider.h"
#include "Crashed/NPC/BaseEnemy.h"
#include "Crashed/NPC/EnemyAIController.h"


// Sets default values
AWaypoint::AWaypoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject <USceneComponent>("Root");
	SetRootComponent(Root);
	Box = CreateDefaultSubobject<UBoxComponent>("Box");
	Box->SetupAttachment(GetRootComponent());
	Box->OnComponentBeginOverlap.AddDynamic(this, &AWaypoint::OnCharacterEnter);
}

void AWaypoint::OnCharacterEnter(UPrimitiveComponent* OverlapComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ASpider* Spider = nullptr;
	
	if (OtherActor != nullptr)
	{
		Spider = Cast<ASpider>(OtherActor);
		if (Spider != nullptr)
		{
			Spider->NextWayPoint = NextWayPoint;
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

