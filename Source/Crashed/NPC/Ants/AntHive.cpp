#include "AntHive.h"
#include "Components/StaticMeshComponent.h"

AAntHive::AAntHive()
{
    PrimaryActorTick.bCanEverTick = false;

    //setup mesh component
    HiveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HiveMesh"));
    RootComponent = HiveMesh;
}

void AAntHive::BeginPlay()
{
    Super::BeginPlay();
}
