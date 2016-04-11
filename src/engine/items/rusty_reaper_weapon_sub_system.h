#ifndef INCLUDED_ENGINE_RUSTY_REAPER_WEAPON_SUB_SYSTEM_H
#define INCLUDED_ENGINE_RUSTY_REAPER_WEAPON_SUB_SYSTEM_H

#include "engine/items/common_sub_system_includes.h"

namespace engine {

class RustyReaperWeaponSubSystem : public SubSystem
{
public:
    DEFINE_SUB_SYSTEM_BASE(RustyReaperWeaponSubSystem)
    RustyReaperWeaponSubSystem();
    virtual void Init();
    virtual void Update( Actor& actor, double DeltaTime );
private:
    Scene& mScene;
    core::ProgramState& mProgramState;
    Opt<WeaponItemSubSystem> mWeaponItemSubSystem;
    ActorFactory& mActorFactory;
    int32_t mProjectileId;
    int32_t mProjectileAltId;
};

} // namespace engine

#endif//INCLUDED_ENGINE_RUSTY_REAPER_WEAPON_SUB_SYSTEM_H

//command:  "classgenerator.exe" -g "weapon" -c "rusty_reaper"
