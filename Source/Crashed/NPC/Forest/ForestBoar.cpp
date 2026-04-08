#include "ForestBoar.h"

#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AForestBoar::AForestBoar()
{
	AggroZone = CreateDefaultSubobject<USphereComponent>(TEXT("AggroZone"));
	AggroZone->SetupAttachment(RootComponent);
	AggroZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AggroZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	AggroZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AForestBoar::BeginPlay()
{
	Super::BeginPlay();

	// Store the Blueprint-configured detection radius, then go passive
	PassiveDetectionRadius = DetectionRadius;
	DetectionRadius = 0.f;

	AggroZone->SetSphereRadius(AggroRadius);
	AggroZone->OnComponentBeginOverlap.AddDynamic(this, &AForestBoar::OnAggroZoneBeginOverlap);
	AggroZone->OnComponentEndOverlap.AddDynamic(this, &AForestBoar::OnAggroZoneEndOverlap);
}

void AForestBoar::OnAggroZoneBeginOverlap(UPrimitiveComponent*, AActor* OtherActor,
	UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (OtherActor && OtherActor == UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		GetWorldTimerManager().ClearTimer(AggroDecayTimer);
		BecomeAggravated();
	}
}

void AForestBoar::OnAggroZoneEndOverlap(UPrimitiveComponent*, AActor* OtherActor,
	UPrimitiveComponent*, int32)
{
	if (OtherActor && OtherActor == UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		GetWorldTimerManager().SetTimer(AggroDecayTimer, this, &AForestBoar::ResetAggro, AggroDecayTime, false);
	}
}

void AForestBoar::BecomeAggravated()
{
	bIsAggravated = true;
	DetectionRadius = HostileDetectionRadius;
}

void AForestBoar::ResetAggro()
{
	bIsAggravated = false;
	DetectionRadius = PassiveDetectionRadius;
}
