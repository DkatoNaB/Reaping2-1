#ifndef INCLUDED_ENGINE_PARTICLE_SYSTEM_H
#define INCLUDED_ENGINE_PARTICLE_SYSTEM_H

#include "core/scene.h"
#include "engine/system.h"

namespace render {

namespace {
namespace nsParticleSystemInit {
bool init();
}
}

class ParticleSystem : public engine::System
{
public:
    DEFINE_SYSTEM_BASE(ParticleSystem)
    ParticleSystem();
protected:
    virtual void Init();
    virtual void Update( double DeltaTime );
private:
    Scene& mScene;
    static bool inited;
    friend bool nsParticleSystemInit::init();
};

namespace {
namespace nsParticleSystemInit {
bool init()
{
    return ParticleSystem::inited;
}
volatile bool gInited=init();
}
}

} // namespace render

#endif//INCLUDED_ENGINE_PARTICLE_SYSTEM_H

//command:  "../../build/tools/classgenerator/classgenerator" -g "system" -c "particle_system" -t "emitter"
