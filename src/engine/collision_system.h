#ifndef INCLUDED_ENGINE_COLLISION_SYSTEM_H
#define INCLUDED_ENGINE_COLLISION_SYSTEM_H

#include "core/collision_store.h"
#include "core/grid.h"
#include "core/scene.h"
#include "engine/system.h"
#include "engine/sub_system_holder.h"
#include "engine/collisions/collision_sub_system.h"
#include "core/perf_timer.h"
#include "core/actor_event.h"

namespace engine {

class CollisionSystem : public System, public SubSystemHolder
{
public:
    DEFINE_SYSTEM_BASE( CollisionSystem )
    CollisionSystem();
    virtual void Init();
    virtual void Update( double DeltaTime );
    std::set<Actor*> GetAllActors( glm::vec2 const& position, double radius, CollisionClass collClass ) const;
    bool IsColliding( Actor const& actor, double radius = 0.0 ) const;
private:
    Opt<CollisionSubSystem> GetCollisionSubSystem( int32_t id );
    Grid mCollisionGrid;
    CollisionStore& mCollisionStore;
    Scene& mScene;
    perf::Timer_t mPerfTimer;
    perf::Timer_t mUpdateTimer;
    AutoReg mOnActorEvent;
    void OnActorEvent( ActorEvent const& Evt );
};

} // namespace engine

#endif//INCLUDED_ENGINE_COLLISION_SYSTEM_H
