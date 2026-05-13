#include "BeeAIController.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

ABeeAIController::ABeeAIController()
{
	// Wider perception for flying units
	AISightRadius    = 800.f;
	AILoseSightRadius = 850.f;
	AIFieldOfView    = 180.f;

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
