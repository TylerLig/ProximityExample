#include "Proximity_Actor.h"
#include "Proximity/Proximity_GameStateComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Items/ItemInstance.h"
#include "Items/Fragments/StaticItemFragment_StaticMesh.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(Proximity_Actor)

FVector AProximity_Actor::DefaultBounds = FVector::ZeroVector;
FVector AProximity_Actor::DefaultScale = FVector(0.4f, 0.4f, 0.4f);

AProximity_Actor::AProximity_Actor(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMeshFinder(TEXT("StaticMesh'/Game/Items/Meshes/SM_DefaultProximityMesh.SM_DefaultProximityMesh'"));
    if(DefaultMeshFinder.Succeeded())
    {
        DefaultMesh = DefaultMeshFinder.Object;
        DefaultBounds = DefaultMesh->GetBounds().BoxExtent *  DefaultScale;
    }

    MeshComponent->SetCollisionProfileName(TEXT("ProximityItem"));

    RootComponent = MeshComponent;
}

FVector AProximity_Actor::GetScaledBounds(UItemInstance* ProvidedItemInstance)
{
    UStaticItemFragment_StaticMesh* StaticMesh_Fragment = ProvidedItemInstance->FindStaticFragmentByClass<UStaticItemFragment_StaticMesh>();
    if(StaticMesh_Fragment && StaticMesh_Fragment->GroundMesh && StaticMesh_Fragment->GroundMeshScale != FVector::ZeroVector)
    {
        return StaticMesh_Fragment->GroundMesh->GetBounds().BoxExtent * StaticMesh_Fragment->GroundMeshScale;
    }
    return DefaultBounds;
}

UProximity_GameStateComponent* AProximity_Actor::GetProximityGameStateComponent() const
{
    AGameStateBase* GameState = UGameplayStatics::GetGameState(GetWorld());
    if(!GameState) return nullptr;

    UProximity_GameStateComponent* ProximityGameStateComponent = GameState->FindComponentByClass<UProximity_GameStateComponent>();
    return ProximityGameStateComponent;
}