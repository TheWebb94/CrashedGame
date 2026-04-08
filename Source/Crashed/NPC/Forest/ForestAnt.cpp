#include "ForestAnt.h"
#include "AntQueen.h"

void AForestAnt::RegisterWithQueen(AAntQueen* InQueen)
{
	Queen = InQueen;
	if (InQueen)
	{
		OnEnemyDeath.AddDynamic(InQueen, &AAntQueen::OnAntDied);
	}
}

void AForestAnt::OnDeath_Implementation()
{
	Super::OnDeath_Implementation();
}
