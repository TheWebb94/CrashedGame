#include "ForestBee.h"
#include "BeeHive.h"

void AForestBee::ActivateSwarmMode()
{
	bIsAngry = true;
	DetectionRadius = AngryDetectionRadius;
}

void AForestBee::SetOwningHive(ABeeHive* InHive)
{
	OwningHive = InHive;
	if (InHive)
	{
		OnEnemyDeath.AddDynamic(InHive, &ABeeHive::OnBeeDied);
	}
}

void AForestBee::OnDeath_Implementation()
{
	// Delegate fires before Destroy() so hive can remove this bee from its list
	Super::OnDeath_Implementation();
}
