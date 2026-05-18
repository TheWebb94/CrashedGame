#include "EnemyAIController.h"

#include "BaseEnemy.h"
#include "Bee/Bee.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "Spider/Spider.h"


AEnemyAIController::AEnemyAIController()
{
	//config for behaviour tree components
	BBC = CreateDefaultSubobject<UBlackboardComponent>("Blackboard");
	BTC = CreateDefaultSubobject<UBehaviorTreeComponent>("BehaviorTree");

	PrimaryActorTick.bCanEverTick = true;

	//config for perception parameters
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("Sight Config");
	SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>("Perception"));

	SightConfig->SightRadius = AISightRadius;
	SightConfig->LoseSightRadius = AILoseSightRadius;
	SightConfig->PeripheralVisionAngleDegrees = AIFieldOfView;
	SightConfig->SetMaxAge(AISightAge);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	//sets sight as the main perception sense and binds detection callback
	GetPerceptionComponent()->SetDominantSense(*SightConfig->GetSenseImplementation());
	GetPerceptionComponent()->OnPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnPawnDetected);
	GetPerceptionComponent()->ConfigureSense(*SightConfig);
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
}

//onPossess — runs the Behavior Tree and initialises the Blackboard
void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	//casts possessed pawn to base enemy so behaviour tree can be accessed
	if (ABaseEnemy* const Enemy = Cast<ABaseEnemy>(InPawn))
	{
		//gets assigned behaviour tree from enemy
		if (UBehaviorTree* const tree = Enemy->GetBehaviorTree())
		{
			//initialises blackboard and starts behaviour tree
			BBC->InitializeBlackboard(*tree->BlackboardAsset);
			Blackboard = BBC;
			BTC->StartTree(*tree);
		}
	}
}

void AEnemyAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//clears player targeting once player is outside sight radius
	if (DistanceToPlayer > AISightRadius)
	{
		//defending bees keep their assigned defensive behaviour
		if (ABee* Bee = Cast<ABee>(GetPawn()))
			if (Bee->bIsDefending) return;

		//don't wipe an explicitly assigned non-player target (e.g. spider)
		APawn* Player = GetWorld()->GetFirstPlayerController()
			? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr;

		//keeps current target if it is not the player
		if (ABaseEnemy* Me = Cast<ABaseEnemy>(GetPawn()))
			if (Me->CurrentAttackTarget && Me->CurrentAttackTarget != Player) return;

		//clears blackboard sight and target values
		BBC->SetValue<UBlackboardKeyType_Bool>("HasLineOfSight", false);
		BBC->ClearValue("TargetActor");
	}
}

FRotator AEnemyAIController::GetControlRotation() const
{
	//reset value if no possessed pawn
	if (GetPawn() == nullptr)
		return FRotator(.0f, 0.f, 0.f);

	//uses pawn yaw as controller rotation
	else
		return FRotator(.0f, GetPawn()->GetActorRotation().Yaw, .0f);
}

void AEnemyAIController::OnPawnDetected(const TArray<AActor*>& DetectedPawns)
{
	//Player detection — highest priority
	for (size_t i = 0; i < DetectedPawns.Num(); i++)
	{
		//checks if detected actor is the player pawn
		if (DetectedPawns[i] == GetWorld()->GetFirstPlayerController()->GetPawn())
		{
			//updates player distance and blackboard target values
			DistanceToPlayer = GetPawn()->GetDistanceTo(DetectedPawns[i]);
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "Detected");
			BBC->SetValue<UBlackboardKeyType_Bool>("HasLineOfSight", true);
			BBC->SetValue<UBlackboardKeyType_Object>("TargetActor", DetectedPawns[i]);

			//stores player as current attack target
			if (ABaseEnemy* Me = Cast<ABaseEnemy>(GetPawn()))
				Me->CurrentAttackTarget = DetectedPawns[i];

			return;
		}
	}

	//spider detection — secondary, only reached if player not found above
	for (size_t i = 0; i < DetectedPawns.Num(); i++)
	{
		//checks if detected actor is a spider
		if (Cast<ASpider>(DetectedPawns[i]))
		{
			//updates blackboard target values for spider target
			BBC->SetValue<UBlackboardKeyType_Bool>("HasLineOfSight", true);
			BBC->SetValue<UBlackboardKeyType_Object>("TargetActor", DetectedPawns[i]);

			//stores spider as current attack target
			if (ABaseEnemy* Me = Cast<ABaseEnemy>(GetPawn()))
				Me->CurrentAttackTarget = DetectedPawns[i];

			break;
		}
	}
}