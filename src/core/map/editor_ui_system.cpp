#include "platform/i_platform.h"
#include "editor_ui_system.h"
#include "ui/ui.h"
#include "../../engine/engine.h"
#include <imgui.h>

namespace map {

EditorUiSystem::EditorUiSystem()
    : mScene( Scene::Get() )
{
}


void EditorUiSystem::Init()
{
    mOnEditorModeChanged = EventServer<map::EditorModeChangedEvent>::Get().Subscribe( boost::bind( &EditorUiSystem::OnEditorModeChanged, this, _1 ) );
    mOnEditorBack = EventServer<map::EditorBackEvent>::Get().Subscribe( boost::bind( &EditorUiSystem::OnEditorBack, this, _1 ) );
}


void EditorUiSystem::Update(double DeltaTime)
{
    if (!mEnabled || !mEditorMode || true)
    {
        return;
    }
    bool shown = true;
    ImGui::Begin( "Editor", &shown );
    char const* targettypes[] = {
        "spawnpoint",
        "mapitem",
        "gun",
        "buff",
        "item",
    };
    ImGui::Combo( "type", &mTargetType, targettypes, 5 );
    char const* targets[] = {
    };
    ImGui::Combo( "target", &mTarget, targets, 0 );
    char const* brush[] = {
        "normal",
        "border",
    };
    ImGui::Combo( "brush", &mBrush, brush, 2 );
    char const* pos[] = {
        "free",
        "snap to grid",
    };
    ImGui::Combo( "positioning", &mPositioning, pos, 2 );
    char const* layers[] = {
        "any",
        "target",
    };
    ImGui::Combo( "select layer", &mSelectLayer, layers, 2 );
    ImGui::End();

}

void EditorUiSystem::OnEditorModeChanged(map::EditorModeChangedEvent const& Evt)
{
    mEditorMode = true;
}

void EditorUiSystem::OnEditorBack( map::EditorBackEvent const& Evt )
{
}

REGISTER_SYSTEM( EditorUiSystem );

} // namespace map

