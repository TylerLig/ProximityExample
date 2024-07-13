#pragma once
#include "GameFramework/Actor.h"
#include "Proximity_Actor.generated.h"

class UItemInstance;
class UProximity_GameStateComponent;
class UStaticMeshComponent;
class UStaticMesh;

UCLASS(Abstract)
class AProximity_Actor : public AActor
{
    GENERATED_BODY()

public:
    AProximity_Actor(const FObjectInitializer& ObjectInitializer);

    static FVector GetScaledBounds(UItemInstance* ProvidedItemInstance);

protected:
    static FVector DefaultBounds;
    static FVector DefaultScale;
    UStaticMesh* DefaultMesh;

    UPROPERTY(EditAnywhere)
    UStaticMeshComponent* MeshComponent;

    UProximity_GameStateComponent* GetProximityGameStateComponent() const;
};