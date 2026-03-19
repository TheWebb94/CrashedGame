#include "BaseWeapon.h"
#include "Components/StaticMeshComponent.h"

ABaseWeapon::ABaseWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;
}
