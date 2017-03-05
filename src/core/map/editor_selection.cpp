#include "editor_selection.h"
#include "core/scene.h"
#include "core/i_renderable_component.h"
#include "core/i_collision_component.h"
#include "core/i_position_component.h"

namespace map {

namespace {
    glm::vec4 actorColor( Actor const& a, ActorColors_t const& curr )
    {
        auto it = curr.find( a.GetGUID() );
        if( it != curr.end() )
        {
            return it->second;
        }
        auto renderableC( a.Get<IRenderableComponent>() );
        auto color = renderableC.IsValid() ? renderableC->GetColor() : glm::vec4( 1.0 );
        return color;
    }
    void setColor( Actor& a, glm::vec4 const& color )
    {
        auto renderableC( a.Get<IRenderableComponent>() );
        if( !renderableC.IsValid() )
        {
            return;
        }
        renderableC->SetColor( color );
    }
}

void EditorSelection::SelectActors( glm::vec2 begin, glm::vec2 end, std::vector<int32_t> const& layers, Mode mode )
{
    LL() << begin.x << begin.y << end.x << end.y << mode;
    ActorColors_t newSelection;
    auto& mScene( Scene::Get() );
    for (auto actor : mScene.GetActors())
    {
        auto positionC( actor->Get<IPositionComponent>() );
        auto renderableC( actor->Get<IRenderableComponent>() );
        if( !renderableC.IsValid() )
        {
            continue;
        }
        if( std::find( layers.begin(), layers.end(), renderableC->GetLayerPriority() ) == layers.end() )
        {
            continue;
        }
        if (positionC.IsValid())
        {
            int32_t x = positionC->GetX();
            int32_t y = positionC->GetY();
            auto collisionC( actor->Get<ICollisionComponent>() );
            int32_t radius = 0;
            if (collisionC.IsValid())
            {
                radius = collisionC->GetRadius();
            }
            if (std::min( begin.x, end.x ) <= x+radius
                && x-radius <= std::max( begin.x, end.x )
                && std::min( begin.y, end.y ) <= y+radius
                && y-radius <= std::max( begin.y, end.y ))
            {
                newSelection[ actor->GetGUID() ] = actorColor( *actor, mSelectedActors );
            }
        }
    }
    LL() << newSelection.size();
    if( mode == Remove )
    {
        std::for_each( newSelection.begin(), newSelection.end(), [&]( std::pair<int32_t,glm::vec4> const& v )
                {
                    auto act = mScene.GetActor( v.first );
                    auto it = mSelectedActors.find( v.first );
                    if( act.IsValid() && it != mSelectedActors.end() )
                    {
                        setColor( *act, it->second );
                        mSelectedActors.erase( it );
                    }
                } );
        return;
    }

    if( mode == ClearAndSelect )
    {
        ClearSelection();
    }
    mSelectedActors.insert( newSelection.begin(), newSelection.end() );

    std::for_each( mSelectedActors.begin(), mSelectedActors.end(), [&]( std::pair<int32_t,glm::vec4> const& v )
            {
                auto act = mScene.GetActor( v.first );
                if( act.IsValid() )
                {
                    setColor( *act, glm::vec4( 0,0,1,1 ) );
                }
            } );

}

void EditorSelection::ClearSelection()
{
    auto& mScene( Scene::Get() );
    std::for_each( mSelectedActors.begin(), mSelectedActors.end(), [&]( std::pair<int32_t,glm::vec4> const& v )
            {
                auto act = mScene.GetActor( v.first );
                if( act.IsValid() )
                {
                    setColor( *act, v.second );
                }
            } );
    mSelectedActors.clear();
}

std::vector<int32_t> EditorSelection::GetSelectedActors() const
{
    std::vector<int32_t> rv;
    std::transform( mSelectedActors.begin(), mSelectedActors.end(), std::back_inserter( rv ), []( std::pair<int32_t,glm::vec4> const& v ) { return v.first; } );
    return rv;
}

} // namespace map

