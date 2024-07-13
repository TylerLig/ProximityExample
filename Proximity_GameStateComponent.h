#pragma once
#include "Proximity/ProximityItem_Actor.h"
#include "Components/GameStateComponent.h"
#include "GameFramework/PlayerState.h"
#include "Proximity/Proximity_ActorComponent.h"
#include "Proximity_GameStateComponent.generated.h"

class AProximity_Actor;
class AProximityItem_Actor;

USTRUCT()
struct FWorldGridCell
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<AProximity_Actor*> ProximityActors;
};

UCLASS()
class UProximity_GameStateComponent : public UGameStateComponent
{
    GENERATED_BODY()
    
protected:
    virtual void BeginPlay() override;

public:
    UProximity_GameStateComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    void AddProximityActor(AProximity_Actor* NewItem);
    UItemInstance* RemoveProximityItemActor(AProximityItem_Actor* ItemActorToRemove);
    AProximityItem_Actor* SpawnProximityItemActor(UItemInstance* ItemInstance, FVector SpawnLocation);

private:
    UPROPERTY()
    float ProximityDistance;
    UPROPERTY()
    float CellSize;
    UPROPERTY()
    TMap<FIntVector, FWorldGridCell> WorldGrid;
    UPROPERTY()
    float UpdateInterval;
    UPROPERTY()
    TArray<APlayerState*> ConnectedPlayerStates;
    UPROPERTY()
    TMap<APlayerState*, FProximityData> LastSentProximityItems;

    bool DrawDebug;

    FIntVector LocationToWorldGridIndex(const FVector& Location) const;
    bool AreProximityItemsEqual(const TArray<AProximity_Actor*>& Items1, const TArray<AProximity_Actor*>& Items2);
    void DrawDebugShapes(const FVector& Location, bool Updated);
    TArray<FIntVector> WorldGridCellIndicesInProximity(const FVector& Location);
    TArray<AProximity_Actor*> FindProximityActorsInProximity(const FVector& Location);

    void AddProximityActors(const TArray<AProximity_Actor*>& ItemActors);

    UFUNCTION()
    void UpdatePlayersProximityItems();

    FTimerHandle TimerHandle_UpdatePlayersProximityItems;
}; 