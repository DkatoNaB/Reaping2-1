#ifndef INCLUDED_ENGINE_ACTOR_SIZE_SYSTEM_H
#define INCLUDED_ENGINE_ACTOR_SIZE_SYSTEM_H

#include "core/scene.h"
#include "engine/system.h"
#include "core/actor_event.h"
#include <map>

namespace engine {

class ActorSizeSystem : public System
{
public:
    DEFINE_SYSTEM_BASE(ActorSizeSystem)
    ActorSizeSystem();
    float GetSize( uint32_t ActorGUID ) const;
protected:
    virtual void Init();
    virtual void Update( double DeltaTime );
private:
    AutoReg mOnActorEvent;
    void OnActorEvent( ActorEvent const& Evt );
    Scene& mScene;
    typedef std::map<uint32_t, float> ActorSizeMap;
    ActorSizeMap mActorSize;
};

} // namespace engine

#endif//INCLUDED_ENGINE_ACTOR_SIZE_SYSTEM_H

//command:  "../../build-rea-release/bin/release/classgenerator" -g "system" -c "actor_size_system" -t "collision"
