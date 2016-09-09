#ifndef INCLUDED_CORE_I_BORDER_COMPONENT_H
#define INCLUDED_CORE_I_BORDER_COMPONENT_H

#include "component.h"
#include "border_type.h"
#include <vector>
#include "platform/export.h"

class IBorderComponent : public Component
{
public:
    DEFINE_COMPONENT_BASE( IBorderComponent )
    typedef std::vector<BorderType::Type> Borders_t;
    typedef std::vector<int32_t> BorderIds_t;
    typedef std::vector<glm::vec2> BorderPositions_t;
    virtual void SetBorders( Borders_t const& borders ) = 0;
    virtual Borders_t const& GetBorders()const = 0;
    virtual BorderIds_t const& GetBorderIds()const = 0;
    virtual void SetOuterBorders( Borders_t const& borders ) = 0;
    virtual Borders_t const& GetOuterBorders()const = 0;
    virtual BorderIds_t const& GetOuterBorderIds()const = 0;
    virtual BorderPositions_t const& GetOuterBorderPositions()const = 0;
    virtual void SetChanged( bool changed ) = 0;
    virtual bool IsChanged()const = 0;
    typedef std::vector<int32_t> RandomSprites_t; //weight of sprites
    virtual void SetRandomSprites( RandomSprites_t const& randomSprites ) = 0;
    virtual RandomSprites_t const& GetRandomSprites()const = 0;
    virtual void SetSpriteIndex( int32_t spriteIndex ) = 0;
    virtual int32_t GetSpriteIndex()const = 0;
public:
    friend class ::boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version );
};

template<class Archive>
void IBorderComponent::serialize( Archive& ar, const unsigned int version )
{
    //NOTE: generated archive for this class
    ar& boost::serialization::base_object<Component>( *this );
}


REAPING2_CLASS_EXPORT_KEY2( IBorderComponent, IBorderComponent, "i_border_component" );
#endif//INCLUDED_CORE_I_BORDER_COMPONENT_H

//command:  "classgenerator.exe" -g "i_component" -c "i_border_component" -m "Borders_t-borders"
