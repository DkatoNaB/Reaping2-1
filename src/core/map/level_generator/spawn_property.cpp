#include "core/map/level_generator/spawn_property.h"
#include "../spawn_actor_map_element.h"
#include "core/position_component.h"
#include "level_generated_map_element.h"
#include "random.h"
#include "../group_map_element.h"


namespace map {

SpawnProperty::SpawnProperty( int32_t Id )
    : IProperty( Id )
    , mTargets()
    , mChance( 100 )
{
}

void SpawnProperty::Load( Json::Value& setters )
{
    IProperty::Load( setters );
    mTargets.clear();
    auto const& targets = setters["targets"];
    if (targets.isArray())
    {
        for (auto& target : targets)
        {
            mTargets.push_back( AutoId( target.asString() ) );
        }
    }
    int32_t chance;
    if (Json::GetInt( setters["chance"], chance ))
    {
        mChance = chance;
    }
}


void SpawnProperty::SetTargets( Targets_t blockedTargets )
{
    mTargets = blockedTargets;
}

SpawnProperty::Targets_t const& SpawnProperty::GetTargets() const
{
    return mTargets;
}



void SpawnProperty::SetChance( int32_t chance )
{
    mChance = chance;
}


int32_t SpawnProperty::GetChance() const
{
    return mChance;
}


void SpawnProperty::Generate( RoomDesc& roomDesc, MapElementHolder mMapElementHolder, glm::vec2 pos )
{
    if (mRand() % 100 < mChance)
    {
        SpawnTargets( roomDesc, mTargets, mMapElementHolder, pos );
    }
}


void SpawnProperty::SpawnTargets( RoomDesc &roomDesc, std::vector<int32_t> targets, MapElementHolder &mMapElementHolder, glm::vec2 &pos )
{
    static int32_t componentId = AutoId( "position_component" );
    static Opt<MapSystem> mapSystem = MapSystem::Get();
    int currTargetIndex = 0;
    while (currTargetIndex<targets.size())
    {
        int32_t target = targets[currTargetIndex];
        for (auto targetMapElement : MapElementListFilter<MapSystem::UID>( mMapElementHolder.mAllMapElements, target ))
        {
            if (targetMapElement->GetType() == SpawnActorMapElement::GetType_static())
            {
                Opt<SpawnActorMapElement> spawnActorMapElement( targetMapElement );
                SpawnActor( spawnActorMapElement, pos );
                MapSystem::Get()->GetMapElementList().insert( targetMapElement );
            }
            else if (targetMapElement->GetType() == GroupMapElement::GetType_static())
            {
                Opt<GroupMapElement> groupMapElement( targetMapElement );
                targets.insert( targets.end(), groupMapElement->GetTargets().begin(), groupMapElement->GetTargets().end() );
            }
        }
        ++currTargetIndex;
    }
}


void SpawnProperty::SpawnActor( Opt<SpawnActorMapElement> spawnActorMapElement, glm::vec2 &pos )
{
    static int32_t componentId = AutoId( "position_component" );
    static Opt<MapSystem> mapSystem = MapSystem::Get();
    Opt<PositionComponentLoader> positionCompLoader = spawnActorMapElement->GetComponentLoader( componentId );
    if (positionCompLoader.IsValid())
    {
        positionCompLoader->Bind<double>( &PositionComponent::AddX, pos.x );
        positionCompLoader->Bind<double>( &PositionComponent::AddY, pos.y );
    }
    for (Opt<LevelGeneratedMapElement> levelGenerated : MapElementListFilter<MapSystem::All>( MapSystem::Get()->GetMapElementList(), LevelGeneratedMapElement::GetType_static() ))
    {
        levelGenerated->PlugInNodeId( LevelGeneratedMapElement::GeneratedNodeId(), spawnActorMapElement->GetInputNodeId( SpawnActorMapElement::SpawnNodeId() ) );
    }
}

} // namespace map
