#include "editor_mode.h"
#include <imgui.h>

namespace map {

EditorMode::EditorMode()
{
}

EditorMode::Mode EditorMode::GetMode() const
{
    return mMode;
}

void EditorMode::SetMode( Mode mode )
{
    mMode = mode;
}

void EditorMode::DrawSelector()
{
    char const* edmodes[] = {
        "actors",
        "entrances",
        "cell fill",
    };
    ImGui::Combo( "mode", reinterpret_cast<int*>(&mMode), edmodes, 3 );
}

}
