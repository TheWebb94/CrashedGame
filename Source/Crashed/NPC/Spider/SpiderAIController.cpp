// Fill out your copyright notice in the Description page of Project Settings.


#include "SpiderAIController.h"

#include "Spider.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Waypoint.h"
#include "Crashed/HealthComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASpiderAIController::ASpiderAIController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("Sight Config");
	SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>("Perception"));
	SightConfig->SightRadius = AISightRadius;
	SightConfig->LoseSightRadius = AILoseSightRadius;
	SightConfig->PeripheralVisionAngleDegrees = AIFieldOfView;
	SightConfig->SetMaxAge(AISightAge);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	GetPerceptionComponent()->SetDominantSense(*SightConfig->GetSenseImplementation());
	GetPerceptionComponent()->OnPerceptionUpdated.AddDynamic(this, &ASpiderAIController::OnPawnDetected);
	GetPerceptionComponent()->ConfigureSense(*SightConfig);
}

void ASpiderAIController::BeginPlay() {
	Super::BeginPlay();
}
void ASpiderAIController::OnPossess(APawn* MyPawn) {
	Super::OnPossess(MyPawn);
}
void ASpiderAIController::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	ASpider* MyCharacter = Cast<ASpider>(GetPawn());
	if (DistanceToOther > AISightRadius)
		bIsPlayerDetected = false;
	if (bIsPlayerDetected) {
		ACharacter* MyPlayer =
		Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(),0));
		MoveToActor(MyPlayer, 5.0f);
	} else if (MyCharacter->NextWayPoint != nullptr)
		MoveToActor(MyCharacter->NextWayPoint, 5.0f);
}
FRotator ASpiderAIController::GetControlRotation() const
{
	if (GetPawn() == nullptr)
		return FRotator(.0f, 0.f, 0.f);
	else
		return FRotator(.0f, GetPawn()->GetActorRotation().Yaw, .0f);
}
void ASpiderAIController::OnPawnDetected(const TArray<AActor*>& DetectedPawns)
{
	for (size_t i = 0; i < DetectedPawns.Num(); i++)
{
	DistanceToOther = GetPawn()->GetDistanceTo(DetectedPawns[i]);
		if (DistanceToOther < AttackRange)
		{
			UHealthComponent* HealthComp = DetectedPawns[i]->FindComponentByClass<UHealthComponent>();
			
			if (HealthComp)
			{
				//add damage stat - this should be from the controlled pawns damage
				HealthComp->ApplyDamage(1.0f);
			}
		}
}
	bIsPlayerDetected = true;
	
}
