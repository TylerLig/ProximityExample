#include "ProximityItem_Actor.h"
#include "Proximity/Proximity_GameStateComponent.h"
#include "Items/ItemInstance.h"
#include "Items/ItemDefinition.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"
#include "Items/Fragments/StaticItemFragment_StaticMesh.h"
#include "Items/Fragments/StaticItemFragment_Icon.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ProximityItem_Actor)

AProximityItem_Actor::AProximityItem_Actor(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
    IsInitialized = false;
}

void AProximityItem_Actor::BeginPlay()
{
    Super::BeginPlay();

    if(!HasAuthority()) return;
    
    if(!ItemInstance && SpawnData.ItemDefinition && SpawnData.StartingItemCount > 0)
    {
        InitializeFromItemDefinition(SpawnData.ItemDefinition, SpawnData.StartingItemCount);
    }
}

void AProximityItem_Actor::InitializeFromItemInstance(UItemInstance* NewItemInstance)
{
    if(IsInitialized || !NewItemInstance) return;
    
    UProximity_GameStateComponent* ProximityGameStateComponent = GetProximityGameStateComponent();
    if(!ProximityGameStateComponent) return;

    ItemInstance = NewItemInstance;
    MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ItemInstance, this);

    ProximityGameStateComponent->AddProximityActor(this);

    IsInitialized = true;
}

void AProximityItem_Actor::InitializeFromItemDefinition(TSubclassOf<UItemDefinition> ItemDefinitionParam, int32 StartingItemCountParam)
{
    if(IsInitialized) return;

    UProximity_GameStateComponent* ProximityGameStateComponent = GetProximityGameStateComponent();
    if(!ProximityGameStateComponent) return;

    InitializeFromItemInstance(UItemInstance::CreateInstanceFromDefinition(ItemDefinitionParam, StartingItemCountParam, ProximityGameStateComponent));
}

void AProximityItem_Actor::OnRep_ItemInstance()
{
    if(!ItemInstance) return;

    UStaticItemFragment_StaticMesh* StaticMesh_Fragment = ItemInstance->FindStaticFragmentByClass<UStaticItemFragment_StaticMesh>();
    bool HasMesh = StaticMesh_Fragment && StaticMesh_Fragment->GroundMesh && StaticMesh_Fragment->GroundMeshScale != FVector::ZeroVector;
    if(HasMesh)
    {
        MeshComponent->SetStaticMesh(StaticMesh_Fragment->GroundMesh);
        MeshComponent->SetRelativeRotation(StaticMesh_Fragment->GroundMeshRotation);
        MeshComponent->SetRelativeScale3D(StaticMesh_Fragment->GroundMeshScale);
    }
    else
    {
        MeshComponent->SetStaticMesh(DefaultMesh);
        MeshComponent->SetRelativeScale3D(DefaultScale);

        //Set the material to the icon texture
        UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("Material'/Game/Items/Meshes/Materials/M_DefaultProximityMaterial.M_DefaultProximityMaterial'"));
        UStaticItemFragment_Icon* Icon_Fragment = ItemInstance->FindStaticFragmentByClass<UStaticItemFragment_Icon>();
        if(Icon_Fragment)
        {
            UObject* TextureObject = Icon_Fragment->Icon.Normal.GetResourceObject();
            UTexture2D* TextureAsset = Cast<UTexture2D>(TextureObject);
            if (TextureAsset && BaseMaterial)
            {
                UMaterialInstanceDynamic* DynamicMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterial, nullptr);
        
                if (DynamicMaterialInstance)
                {
                    DynamicMaterialInstance->SetTextureParameterValue(FName("BaseColor"), TextureAsset);
                    MeshComponent->SetMaterial(0, DynamicMaterialInstance);
                }
            }
        }
    }
}

void AProximityItem_Actor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ItemInstance, Params);
}

bool AProximityItem_Actor::ReplicateSubobjects(UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
    if(ItemInstance && IsValid(ItemInstance))
    {
        TArray<UItemFragment*> InstanceFragments = ItemInstance->GetAllFragments();
        for(UItemFragment* Fragment : InstanceFragments)
        {
            WroteSomething |= Channel->ReplicateSubobject(Fragment, *Bunch, *RepFlags);
        }
        WroteSomething |= Channel->ReplicateSubobject(ItemInstance, *Bunch, *RepFlags);
    }
	return WroteSomething;
}