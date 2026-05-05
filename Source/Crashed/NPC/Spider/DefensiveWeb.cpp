#include "DefensiveWeb.h"

#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

ADefensiveWeb::ADefensiveWeb()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    TriggerBox->SetBoxExtent(FVector(80.f, 80.f, 20.f));
    TriggerBox->SetCollisionProfileName(TEXT("OverlapAll"));
    RootComponent = TriggerBox;

    WebMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WebMesh"));
    WebMesh->SetupAttachment(TriggerBox);
    WebMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ADefensiveWeb::BeginPlay()
{
    Super::BeginPlay();
    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ADefensiveWeb::OnOverlapBegin);
    SetLifeSpan(WebLifespan);
}

//entagles overlapping otherActor
void ADefensiveWeb::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == GetOwner()) return;
    if (EntangledActors.Contains(OtherActor)) return; //prevents attempting to entangle an already entangled target

    EntangleTarget(OtherActor);
}

//sets maxmovespeed of target to zero, then clears after timer depletes
void ADefensiveWeb::EntangleTarget(AActor* Target)
{
    ACharacter* Char = Cast<ACharacter>(Target);
    if (!Char) return; //safety check

    UCharacterMovementComponent* MoveComp = Char->GetCharacterMovement();
    if (!MoveComp || MoveComp->MaxWalkSpeed <= 0.f) return;

    EntangledActors.Add(Target);

    float OriginalSpeed = MoveComp->MaxWalkSpeed;
    MoveComp->MaxWalkSpeed = 0.f; 

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