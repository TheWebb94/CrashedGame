#include "DefensiveWeb.h"

#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

ADefensiveWeb::ADefensiveWeb()
{
    PrimaryActorTick.bCanEverTick = false;

    //setup triggerboxes in obj
    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    TriggerBox->SetBoxExtent(FVector(80.f, 80.f, 20.f));
    TriggerBox->SetCollisionProfileName(TEXT("OverlapAll"));
    RootComponent = TriggerBox;

    //setup mesh in obj
    WebMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WebMesh"));
    WebMesh->SetupAttachment(TriggerBox);
    WebMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ADefensiveWeb::BeginPlay()
{
    Super::BeginPlay();
    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ADefensiveWeb::OnOverlapBegin); //bind an event to overlapping anotherr actor
    SetLifeSpan(WebLifespan); //automatically destroys the actor after lifespan is reached
}

//entagles overlapping otherActor
void ADefensiveWeb::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == GetOwner()) return; //prevents entangling self, or other invalid targets
    if (EntangledActors.Contains(OtherActor)) return; //prevents attempting to entangle an already entangled target

    EntangleTarget(OtherActor); //applies the entangle
}

//sets maxmovespeed of target to zero, then clears after timer depletes
void ADefensiveWeb::EntangleTarget(AActor* Target)
{
    ACharacter* Char = Cast<ACharacter>(Target);
    if (!Char) return; //validity check

    UCharacterMovementComponent* MoveComp = Char->GetCharacterMovement(); //get the targets movement component
    if (!MoveComp || MoveComp->MaxWalkSpeed <= 0.f) return; //safety checks

    EntangledActors.Add(Target); //track all entangled targets (they are prioritised by the spider in their utility AI)

    float OriginalSpeed = MoveComp->MaxWalkSpeed;
    MoveComp->MaxWalkSpeed = 0.f; //sert movement speed to zero while entangled

    //set a timer so that entangled status can be removed
    TWeakObjectPtr<UCharacterMovementComponent> WeakMove(MoveComp);
    TWeakObjectPtr<AActor> WeakTarget(Target);
    FTimerHandle Handle;
    GetWorldTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([this, WeakMove, WeakTarget, OriginalSpeed]()
    {
        if (WeakMove.IsValid())
            WeakMove->MaxWalkSpeed = OriginalSpeed;
        if (WeakTarget.IsValid())
            EntangledActors.Remove(WeakTarget.Get());
    }), EntangleDuration, false);
}