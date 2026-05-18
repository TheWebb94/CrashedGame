#include "BeeAIController.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

ABeeAIController::ABeeAIController()
{
	//config
	AISightRadius    = 800.f;
	AILoseSightRadius = 850.f;
	AIFieldOfView    = 180.f; //high FOV from bees, because its a swarm of bees

	// Reconfigure sight with updated values
	if (SightConfig)
	{
		SightConfig->SightRadius                  = AISightRadius;
		SightConfig->LoseSightRadius              = AILoseSightRadius;
		SightConfig->PeripheralVisionAngleDegrees = AIFieldOfView;
		if (GetPerceptionComponent())
			GetPerceptionComponent()->ConfigureSense(*SightConfig);
	}
}
