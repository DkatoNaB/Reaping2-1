#include "normal_brush.h"
#include "spawn_actor_map_element.h"
#include "map_system.h"
#include "map_element_factory.h"
#include "engine/engine.h"
#include "input/mouse.h"
#include "../position_component.h"
#include "editor_target_system.h"
#include "../i_collision_component.h"
#include "spawn_actor_map_element_system.h"
#include "engine/system.h"
namespace map {

NormalBrush::NormalBrush( int32_t Id )
    : IBrush( Id )
{

}

void NormalBrush::CreateTarget()
{
    RemoveWhenUsedRAII( false );
    EditorTargetSystem::Get()->PutTarget( EditorTargetSystem::Get()->GetCursorPosition() );
    Opt<engine::System> spawnActorMES( engine::Engine::Get().GetSystem<SpawnActorMapElementSystem>() );
    spawnActorMES->Update( 0 );
}

void NormalBrush::RemoveTarget()
{
    std::vector<int32_t> removeActors = GetActorsToRemove();

    for ( std::vector<int32_t>::iterator i = removeActors.begin(), e = removeActors.end(); i != e; ++i )
    {
        mScene.RemoveActor( *i );
        MapSystem::Get()->RemoveMapElement( *i );
    }
}

} // namespace map

