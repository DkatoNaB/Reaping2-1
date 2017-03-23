#ifndef INCLUDED_MAP_EDITOR_CAMERA_SYSTEM_H
#define INCLUDED_MAP_EDITOR_CAMERA_SYSTEM_H

#include "core/scene.h"
#include "engine/system.h"
#include "input/keyboard.h"
#include "input/mouse.h"
#include "engine/system.h"
#include "render/renderer.h"
#include "input/keyboard.h"
#include "input/mouse.h"

namespace map {

class EditorCameraSystem : public engine::System
{
public:
    DEFINE_SYSTEM_BASE(EditorCameraSystem)
    EditorCameraSystem();
    double const& GetX() const;
    double const& GetY() const;
    static Opt<EditorCameraSystem> Get();
protected:
    virtual void Init();
    virtual void Update( double DeltaTime );
private:
    AutoReg mOnScreenMouseMove;
    void OnScreenMouseMove( ::ScreenMouseMoveEvent const& Evt );
    double mX = 0;
    double mY = 0;
    uint32_t mCurrentMovement = 0;

};

INSTANTIATE_SYSTEM( EditorCameraSystem );
} // namespace map

#endif//INCLUDED_MAP_EDITOR_CAMERA_SYSTEM_H

//command:  "../../../build-reap/bin/classgenerator" -g "system" -c "editor_camera_system" -n "map"
