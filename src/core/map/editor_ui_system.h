#ifndef INCLUDED_MAP_EDITOR_UI_SYSTEM_H
#define INCLUDED_MAP_EDITOR_UI_SYSTEM_H

#include "core/scene.h"
#include "engine/system.h"
#include "editor_mode_changed_event.h"
#include "editor_back_event.h"

namespace map {

class EditorUiSystem : public engine::System
{
public:
    DEFINE_SYSTEM_BASE(EditorUiSystem)
    EditorUiSystem();
protected:
    virtual void Init();
    virtual void Update( double DeltaTime );
private:
    Scene& mScene;
    AutoReg mOnEditorModeChanged;
    void OnEditorModeChanged(map::EditorModeChangedEvent const& Evt);

    AutoReg mOnEditorBack;
    void OnEditorBack( map::EditorBackEvent const& Evt );

    bool mEditorMode = false;
    int mTargetType = 0;
    int mTarget = 0;
    int mBrush = 0;
    int mPositioning = 0;
    int mSelectLayer = 0;
};

INSTANTIATE_SYSTEM( EditorUiSystem );

} // namespace map

#endif//INCLUDED_MAP_EDITOR_UI_SYSTEM_H


