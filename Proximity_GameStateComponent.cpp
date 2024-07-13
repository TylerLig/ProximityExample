#include "Proximity_GameStateComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Items/ItemInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(Proximity_GameStateComponent)

UProximity_GameStateComponent::UProximity_GameStateComponent(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = false;
    ProximityDistance = 200.0f;
    CellSize = 1000.0f;
    UpdateInterval = 0.25f;
    DrawDebug = false;
}

void UProximity_GameStateComponent::BeginPlay()
{
    Super::BeginPlay();
    if (!HasAuthority()) return;

    GetWorld()->GetTimerManager().SetTimer(TimerHandle_UpdatePlayersProximityItems, this, &UProximity_GameStateComponent::UpdatePlayersProximityItems, UpdateInterval, true);    
}

TArray<AProximity_Actor*> UProximity_GameStateComponent::FindProximityActorsInProximity(const FVector& Location)
{
    TArray<AProximity_Actor*> Output;
    TArray<FIntVector> WorldGridCells = WorldGridCellIndicesInProximity(Location);
    for (FIntVector WorldGridCellIndex : WorldGridCells)
    {
        FWorldGridCell* WorldGridCell = WorldGrid.Find(WorldGridCellIndex);
        if (!WorldGridCell) continue;
        
        for (AProximity_Actor* ProximityActor : WorldGridCell->ProximityActors)
        {
            if (FVector::Dist(ProximityActor->GetActorLocation(), Location) > ProximityDistance) continue;
            
            Output.Add(ProximityActor);     
        }
    }
    Output.Sort([](const AProximity_Actor& A, const AProximity_Actor& B) {
        return A.GetName() < B.GetName(); //sorted by name for consistent ordering
    });
    return Output;
}

AProximityItem_Actor* UProximity_GameStateComponent::SpawnProximityItemActor(UItemInstance* ItemInstance, FVector SpawnLocation)
{
    if (!ItemInstance || !HasAuthority()) return nullptr;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AProximityItem_Actor* NewActor = GetWorld()->SpawnActor<AProximityItem_Actor>(AProximityItem_Actor::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);
    NewActor->InitializeFromItemInstance(ItemInstance);

    return NewActor;
}

void UProximity_GameStateComponent::AddProximityActor(AProximity_Actor* ProximityActor)
{
    if (!ProximityActor) return;

    AProximityItem_Actor* ProximityItemActor = Cast<AProximityItem_Actor>(ProximityActor);
    if(ProximityItemActor && !ProximityItemActor->ItemInstance) return;

    FIntVector GridCoords = LocationToWorldGridIndex(ProximityActor->GetActorLocation());
    FWorldGridCell& Cell = WorldGrid.FindOrAdd(GridCoords, FWorldGridCell());
    Cell.ProximityActors.Add(ProximityActor);
}

void UProximity_GameStateComponent::AddProximityActors(const TArray<AProximity_Actor*>& ProximityActors)
{
    for (AProximity_Actor* ProximityActor : ProximityActors)
    {
        AddProximityActor(ProximityActor);
    }
}

UItemInstance* UProximity_GameStateComponent::RemoveProximityItemActor(AProximityItem_Actor* ItemActorToRemove)
{
    if (!ItemActorToRemove) return nullptr;

    const FVector& ItemLocation = ItemActorToRemove->GetActorLocation();
    
    FIntVector GridCoords = LocationToWorldGridIndex(ItemLocation);
    FWorldGridCell& Cell = WorldGrid.FindOrAdd(GridCoords, FWorldGridCell());

    UItemInstance* ItemInstance = ItemActorToRemove->ItemInstance;
    if(!ItemInstance) return nullptr;
    int32 RemovedElements = Cell.ProximityActors.Remove(ItemActorToRemove);
    if(RemovedElements == 0) return nullptr;

    ItemActorToRemove->Destroy();
    
    return ItemInstance;
}

FIntVector UProximity_GameStateComponent::LocationToWorldGridIndex(const FVector& Location) const
{
    FIntVector GridCoords;
    GridCoords.X = FMath::FloorToInt(Location.X / CellSize);
    GridCoords.Y = FMath::FloorToInt(Location.Y / CellSize);
    GridCoords.Z = FMath::FloorToInt(Location.Z / CellSize);
    return GridCoords;
}

void UProximity_GameStateComponent::UpdatePlayersProximityItems()
{
    AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
    if(!GameState) return;

    for (APlayerState* PlayerState : GameState->PlayerArray)
    {
        APawn* PlayerPawn = PlayerState->GetPawn();
        if (!PlayerPawn) continue;

        FVector PlayerLocation = PlayerPawn->GetActorLocation();

        TArray<AProximity_Actor*> ProximityActors = FindProximityActorsInProximity(PlayerLocation);
        bool ShouldUpdateProximityItemsForPlayer = 
            !LastSentProximityItems.Contains(PlayerState) || 
            !AreProximityItemsEqual(
                LastSentProximityItems[PlayerState].ProximityActors,
                ProximityActors
            );

        if(DrawDebug)
        {
            DrawDebugShapes(PlayerLocation, ShouldUpdateProximityItemsForPlayer);
        }

        if (!ShouldUpdateProximityItemsForPlayer) continue;
        
        AController* PlayerController = Cast<AController>(PlayerState->GetOwner());
        if (!PlayerController) continue;

        UProximity_ActorComponent* ProximityActorComponent = PlayerController->FindComponentByClass<UProximity_ActorComponent>();
        if (!ProximityActorComponent) continue;

        FProximityData ProximityData;
        ProximityData.ProximityActors = ProximityActors;
        ProximityActorComponent->UpdateProximityItems(ProximityData);
        LastSentProximityItems.Add(PlayerState, ProximityData);
    }
}

bool UProximity_GameStateComponent::AreProximityItemsEqual(const TArray<AProximity_Actor*>& Items1, const TArray<AProximity_Actor*>& Items2)
{
    if (Items1.Num() != Items2.Num())
    {
        return false;
    }

    for (int32 Index = 0; Index < Items1.Num(); Index++)
    {
        if (Items1[Index] != Items2[Index])
        {
            return false;
        }
    }

    return true;
}

TArray<FIntVector> UProximity_GameStateComponent::WorldGridCellIndicesInProximity(const FVector& Location)
{
    TArray<FIntVector> Output;
    FIntVector WorldGridIndex = LocationToWorldGridIndex(Location);
    Output.Add(WorldGridIndex);

    FVector MinBound = FVector(WorldGridIndex) * CellSize;
    FVector MaxBound = MinBound + FVector(CellSize);

    if (Location.X - MinBound.X <= ProximityDistance) Output.Add(WorldGridIndex + FIntVector(-1, 0, 0));
    if (Location.Y - MinBound.Y <= ProximityDistance) Output.Add(WorldGridIndex + FIntVector(0, -1, 0));
    if (Location.Z - MinBound.Z <= ProximityDistance) Output.Add(WorldGridIndex + FIntVector(0, 0, -1));
    if (MaxBound.X - Location.X <= ProximityDistance) Output.Add(WorldGridIndex + FIntVector(1, 0, 0));
    if (MaxBound.Y - Location.Y <= ProximityDistance) Output.Add(WorldGridIndex + FIntVector(0, 1, 0));
    if (MaxBound.Z - Location.Z <= ProximityDistance) Output.Add(WorldGridIndex + FIntVector(0, 0, 1));

    return Output;
}

void UProximity_GameStateComponent::DrawDebugShapes(const FVector& Location, bool Updated)
{
    TArray<FIntVector> WorldGridCellIndices = WorldGridCellIndicesInProximity(Location);
    for (FIntVector WorldGridCellIndex : WorldGridCellIndices)
    {
        FVector CellMin = FVector(WorldGridCellIndex) * CellSize;
        FVector CellMax = CellMin + FVector(CellSize);
        FColor CellColor = Updated ? FColor::Yellow : FColor::Green;
        DrawDebugBox(GetWorld(), CellMin + FVector(CellSize) / 2, FVector(CellSize) / 2, CellColor, false, 0.5, 0, 2);
    }

    FColor SphereColor = Updated ? FColor::Green : FColor::Red;
    DrawDebugSphere(GetWorld(), Location, ProximityDistance, 32, SphereColor, false, 0.5, 0, 2);
}