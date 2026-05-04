#include "EnemyAIController.h"

#include "BaseEnemy.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"


AEnemyAIController::AEnemyAIController()
{
	BBC = CreateDefaultSubobject<UBlackboardComponent>("Blackboard");
	BTC = CreateDefaultSubobject<UBehaviorTreeComponent>("BehaviorTree");

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
	GetPerceptionComponent()->OnPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnPawnDetected);
	GetPerceptionComponent()->ConfigureSense(*SightConfig);

}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
}

// OnPossess — runs the Behavior Tree and initialises the Blackboard
void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (ABaseEnemy* const Enemy = Cast<ABaseEnemy>(InPawn))
	{
		if (UBehaviorTree* const tree = Enemy->GetBehaviorTree())
		{
			
			BBC->InitializeBlackboard(*tree->BlackboardAsset);
			Blackboard = BBC;
			BTC->StartTree(*tree);
			
		}
	}
}

void AEnemyAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (DistanceToPlayer > AISightRadius)
	{
		BBC->SetValue<UBlackboardKeyType_Bool>("HasLineOfSight", false);
		BBC->ClearValue("TargetActor");
	}

}

FRotator AEnemyAIController::GetControlRotation() const
{
	if (GetPawn() == nullptr)
		return FRotator(.0f, 0.f, 0.f);
	else
		return FRotator(.0f, GetPawn()->GetActorRotation().Yaw, .0f);

}

void AEnemyAIController::OnPawnDetected(const TArray<AActor*>& DetectedPawns)
{
	for (size_t i = 0; i < DetectedPawns.Num(); i++)
		if (DetectedPawns[i] == GetWorld()->GetFirstPlayerController()->GetPawn())
		{
			DistanceToPlayer = GetPawn()->GetDistanceTo(DetectedPawns[i]);
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "Detected");
			BBC->SetValue<UBlackboardKeyType_Bool>("HasLineOfSight", true);
			BBC->SetValue<UBlackboardKeyType_Object>("TargetActor", DetectedPawns[i]);
		}

}


