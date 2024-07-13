#pragma once
#include "Proximity/Proximity_Actor.h"
#include "ProximityItem_Actor.generated.h"

class UItemDefinition;
class UItemInstance;

USTRUCT()
struct FProximityItem_SpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    TSubclassOf<UItemDefinition> ItemDefinition;

    UPROPERTY(EditAnywhere)
    int32 StartingItemCount;
};
UCLASS()
class AProximityItem_Actor : public AProximity_Actor
{
    GENERATED_BODY()

public:
    AProximityItem_Actor(const FObjectInitializer& ObjectInitializer);

    void InitializeFromItemInstance(UItemInstance* NewItemInstance);
    void InitializeFromItemDefinition(TSubclassOf<UItemDefinition> ItemDefinitionParam, int32 StartingItemCountParam);

    virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

    UPROPERTY(ReplicatedUsing = OnRep_ItemInstance)
    UItemInstance* ItemInstance;

protected:
    virtual void BeginPlay() override;

private:
    UFUNCTION()
    void OnRep_ItemInstance();

    bool IsInitialized;

    UPROPERTY(EditAnywhere)
    FProximityItem_SpawnData SpawnData;
};