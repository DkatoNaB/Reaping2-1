#include "core/border_component.h"
#include "platform/id_storage.h"
#include <portable_iarchive.hpp>
#include <portable_oarchive.hpp>
#include "actor.h"

BorderComponent::BorderComponent()
    : mBorders()
    , mChanged(false)
{
}

void BorderComponent::SetBorders( Borders_t const& borders )
{
    mBorders = borders;
    UpdateForActor( mActor );
}

IBorderComponent::Borders_t const& BorderComponent::GetBorders()const
{
    return mBorders;
}

void BorderComponent::SetChanged( bool changed )
{
    mChanged = changed;
}

bool BorderComponent::IsChanged() const
{
    return mChanged;
}

void BorderComponent::SetRandomSprites( RandomSprites_t const& randomSprites )
{
    mRandomSprites = randomSprites;
}

IBorderComponent::RandomSprites_t const& BorderComponent::GetRandomSprites() const
{
    return mRandomSprites;
}

void BorderComponent::SetSpriteIndex( int32_t spriteIndex )
{
    mSpriteIndex = spriteIndex;
}

int32_t BorderComponent::GetSpriteIndex() const
{
    return mSpriteIndex;
}


void BorderComponent::Save( Json::Value& component )
{
    Component::Save( component );
    Json::Value SettersArr( Json::arrayValue );
    Json::Value Setters( Json::objectValue );
    IdStorage& idStorage = IdStorage::Get();
    BorderType& borderType = BorderType::Get();
    {
        Json::Value BordersArr( Json::arrayValue );
        for ( IBorderComponent::Borders_t::iterator i = mBorders.begin(), e = mBorders.end(); i != e; ++i )
        {
            std::string borderName;
            if ( idStorage.GetName( borderType( *i ), borderName ) )
            {
                Json::Value jName = Json::Value( borderName );
                BordersArr.append( jName );
            }
        }
        Setters["borders"] = BordersArr;
    }
    {
        Json::Value BordersArr( Json::arrayValue );
        for ( IBorderComponent::Borders_t::iterator i = mOuterBorders.begin(), e = mOuterBorders.end(); i != e; ++i )
        {
            std::string borderName;
            if ( idStorage.GetName( borderType( *i ), borderName ) )
            {
                Json::Value jName = Json::Value( borderName );
                BordersArr.append( jName );
            }
        }
        Setters["outer_borders"] = BordersArr;
    }
    SettersArr.append( Setters );

    component["set"] = SettersArr;
}

void BorderComponent::SetOuterBorders( Borders_t const& borders )
{
    mOuterBorders = borders;
    UpdateForActor( mActor );
}

IBorderComponent::Borders_t const& BorderComponent::GetOuterBorders() const
{
    return mOuterBorders;
}

IBorderComponent::BorderIds_t const& BorderComponent::GetOuterBorderIds() const
{
    return mOuterBorderIds;
}

IBorderComponent::BorderPositions_t const& BorderComponent::GetOuterBorderPositions() const
{
    return mOuterBorderPositions;
}

IBorderComponent::BorderIds_t const& BorderComponent::GetBorderIds() const
{
    return mBorderIds;
}

void BorderComponent::UpdateForActor( Actor* actor )
{
    if( actor != nullptr )
    {
        mActor = actor;
    }
    if( mActor == nullptr )
    {
        return;
    }

    mBorderIds.clear();
    mOuterBorderIds.clear();
    static BorderType& mBorderType = BorderType::Get();
    static IdStorage& mIdStorage = IdStorage::Get();

    std::string actorName;
    bool const gotId = mIdStorage.GetName( mActor->GetId(), actorName );
    BOOST_ASSERT( gotId );

    for ( auto const& border : mBorders )
    {
        std::string borderName;
        if( mIdStorage.GetName( mBorderType( border ), borderName ) )
        {
            mBorderIds.push_back( mIdStorage.GetId( actorName + "_" + borderName ) );
        }
    }
    for ( auto const& border : mOuterBorders )
    {
        std::string borderName;
        if( mIdStorage.GetName( mBorderType( border ), borderName ) )
        {
            mOuterBorderIds.push_back( mIdStorage.GetId( actorName + "_" + borderName + "_outer") );
            mOuterBorderPositions.push_back( mBorderType.GetNeighborDirs()[ border ] );
        }
    }
}


void BorderComponentLoader::FillProperties( ComponentHolder& holder ) const
{
    ComponentLoader<BorderComponent>::FillProperties( holder );
    Actor* actor = dynamic_cast<Actor*>( &holder );
    if( actor == nullptr )
    {
        return;
    }
    Opt<BorderComponent> borderC = actor->Get<BorderComponent>();
    BOOST_ASSERT( borderC.IsValid() );
    // set the border ids
    borderC->UpdateForActor( actor );
}

void BorderComponentLoader::BindValues()
{
    {
        Json::Value const& json = ( *mSetters )["borders"];
        if( json.isArray() )
        {
            IBorderComponent::Borders_t borders;
            for( Json::Value::iterator i = json.begin(), e = json.end(); i != e; ++i )
            {
                Json::Value& part = *i;
                BorderType::Type typ = BorderType::Get()( AutoId( part.asString() ) );
                borders.push_back( typ );
            }
            Bind<IBorderComponent::Borders_t>( &BorderComponent::SetBorders, borders );
        }
    }
    {
        Json::Value const& json = ( *mSetters )["outer_borders"];
        if( json.isArray() )
        {
            IBorderComponent::Borders_t borders;
            for( Json::Value::iterator i = json.begin(), e = json.end(); i != e; ++i )
            {
                Json::Value& part = *i;
                BorderType::Type typ = BorderType::Get()( AutoId( part.asString() ) );
                borders.push_back( typ );
            }
            Bind<IBorderComponent::Borders_t>( &BorderComponent::SetOuterBorders, borders );
        }
    }
    IBorderComponent::RandomSprites_t randomSprites;
    auto const& json = (*mSetters)["random_sprites"];
    if (json.isArray()&&!json.empty())
    {
        for (auto& chance : json)
        {
            randomSprites.push_back( chance.asInt() );
        }
        Bind<IBorderComponent::RandomSprites_t>( &BorderComponent::SetRandomSprites, randomSprites );
    }
}

BorderComponentLoader::BorderComponentLoader()
{
}

REAPING2_CLASS_EXPORT_IMPLEMENT( BorderComponent, BorderComponent );
