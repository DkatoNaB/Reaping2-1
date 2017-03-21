#include "property_editor.h"
#include "level_generator/property_factory.h"
#include <imgui.h>

namespace map {

PropertyEditor::PropertyEditor()
{
}

void PropertyEditor::DrawUI()
{
    bool shown = true;
    ImGui::Begin( "Property", &shown );
    auto const& keys = PropertyFactory::Get().Keys();
    auto const& keystrings = PropertyFactory::Get().KeyStrings();
    std::vector<char const*> keycstrs;
    std::transform( keystrings.begin(), keystrings.end(), std::back_inserter( keycstrs ),
            []( std::string const& v ){ return v.c_str(); } );
    ImGui::Combo( "new prop", &mNewPropType, &keycstrs[0], keycstrs.size() );

    DrawExistingProperties();

    ImGui::End();
}

void PropertyEditor::DrawExistingProperties()
{
}

}

