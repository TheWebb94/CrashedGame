#include "AntHive.h"
#include "Components/StaticMeshComponent.h"

AAntHive::AAntHive()
{
    PrimaryActorTick.bCanEverTick = false;

    HiveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HiveMesh"));
    RootComponent = HiveMesh;
}

void AAntHive::BeginPlay()
{
    Super::BeginPlay();
}
