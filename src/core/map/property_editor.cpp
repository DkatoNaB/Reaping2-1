#include "property_editor.h"
#include "level_generator/property_factory.h"
#include "level_generator/i_room.h"
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
    ImGui::SameLine();
    static std::string buttonStr( "create" );
    ImGui::Button( buttonStr.c_str() );
    if (ImGui::IsItemClicked())
    {
        CreateNewProperty();
    }

    DrawExistingProperties();

    DrawSelectedPropertySettings();

    ImGui::End();
}

void PropertyEditor::DrawSelectedPropertySettings()
{
    if (!mRoomDesc.IsValid())
    {
        return;
    }
    ImGui::Separator();
    auto& props = mRoomDesc->GetRoom()->GetProperties();
    auto& prop = props.at( mCurrentProperty );
    prop.DrawUI();
}

void PropertyEditor::DrawExistingProperties()
{
    if (!mRoomDesc.IsValid())
    {
        return;
    }
    ImGui::Separator();
    auto const& props = mRoomDesc->GetRoom()->GetProperties();
    std::vector<std::string> strprops;
    for (auto const& prop : props)
    {
        int32_t id = prop.GetId();
        int32_t uid = prop.GetUID();
        int32_t type = prop.GetType();
        std::string idstr;
        std::string typestr;
        static platform::IdStorage& ids(platform::IdStorage::Get());
        if (ids.GetName(id, idstr) && ids.GetName(type, typestr))
        {
            strprops.push_back(idstr + "(" + typestr + ")");
        }
        else
        {
            strprops.push_back("<no id>");
        }
    }

    std::vector<char const*> keycstrs;
    std::transform( strprops.begin(), strprops.end(), std::back_inserter( keycstrs ),
            []( std::string const& v ){ return v.c_str(); } );
    ImGui::Combo( "existing prop", &mCurrentProperty, &keycstrs[0], keycstrs.size() );
}

void PropertyEditor::SetRoomDesc( RoomDesc* desc )
{
    mRoomDesc = desc;
}

void PropertyEditor::CreateNewProperty()
{
    if (!mRoomDesc.IsValid())
    {
        return;
    }

    static auto& propertyFactory = PropertyFactory::Get();
    auto const& keys = PropertyFactory::Get().Keys();
    int32_t propertyId = keys.at(mNewPropType);
    auto prop = propertyFactory(propertyId);
    mRoomDesc->GetRoom()->AddProperty(prop);
}

}

