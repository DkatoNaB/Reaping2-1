#include "core/map/level_generator/i_property.h"
#include <imgui.h>

namespace map {

IProperty::IProperty( int32_t Id )
    : mId( Id )
{
    static int32_t NextUID = 0;
    mUID = ++NextUID;
}

void IProperty::Load( Json::Value const& setters )
{
}


void IProperty::Save( Json::Value& setters ) const
{
    auto& idStorage = IdStorage::Get();
    std::string propName;
    if (idStorage.GetName( mId, propName ))
    {
        Json::Value jName = Json::Value( propName );
        setters["name"] = jName;
    }

}

int32_t IProperty::GetId() const
{
    return mId;
}

int IProperty::GetType() const
{
    return 0;
}

void IProperty::DrawUI()
{
    int32_t id = GetId();
    int32_t uid = GetUID();
    int32_t type = GetType();
    std::string idstr;
    std::string typestr;
    static platform::IdStorage& ids(platform::IdStorage::Get());
    std::string name;
    if (ids.GetName(id, idstr) && ids.GetName(type, typestr))
    {
        name = idstr + "(" + typestr + ")";
    }
    else
    {
        name = "<no id>";
    }
    ImGui::Text( "Prop: %s", name.c_str() );
}

void IProperty::Generate( RoomDesc& roomDesc, MapElementHolder& mMapElementHolder, glm::vec2 pos, bool editor/*= false*/)
{
}


int32_t IProperty::GetUID() const
{
    return mUID;
}

DefaultProperty::DefaultProperty( int32_t Id )
    : IProperty( Id )
{

}

} // namespace map
