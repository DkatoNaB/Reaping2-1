#include "render/border_action_renderer.h"
#include "core/border_type.h"
#include "core/i_border_component.h"
#include "platform/id_storage.h"
#include "core/i_collision_component.h"
#include "idle_action_renderer.h"
#include "engine/engine.h"
#include "engine/actor_size_system.h"

namespace render {

BorderActionRenderer::BorderActionRenderer( int32_t Id )
    : ActionRenderer( Id )
    , mActionId( AutoId( "body_idle" ) )
    , mActorSize( 1.0 )
{
}


void BorderActionRenderer::Init( Actor const& actor )
{
    mSprites.clear();
    static IdStorage& mIdStorage = IdStorage::Get();
    Opt<IBorderComponent> borderC = actor.Get<IBorderComponent>();
    if ( !borderC.IsValid() )
    {
        // this should be valid. border_recognizer guarantees it. Other recognizers can be paired with this renderer
        BOOST_ASSERT( borderC.IsValid() );
        return;
    }
    auto const& borderIds = borderC->GetBorderIds();
    auto const& outerBorderIds = borderC->GetOuterBorderIds();
    auto const& outerBorderPositions = borderC->GetOuterBorderPositions();
    double scale = 1.0;
    SpriteCollection const& Sprites = mRenderableRepo( actor.GetId() );
    Sprite const& Spr = Sprites( mActionId );
    if( Spr.IsValid() )
    {
        scale = Spr.GetScale() * 1.0;
    }

    static engine::ActorSizeSystem& ass( *engine::Engine::Get().GetSystem<engine::ActorSizeSystem>() );
    mActorSize = ass.GetSize( actor.GetGUID() ) * scale;

    for ( auto const& borderId : borderIds )
    {
        auto const& id = IdleActionRenderer::GetSpriteId( borderC->GetSpriteIndex(), borderId );
        SpriteCollection const& Sprites = mRenderableRepo( id );
        Sprite const& Spr = Sprites( mActionId );
        if( Spr.IsValid() )
        {
            SpritePhase const& Phase = Spr( ( int32_t )GetState() );
            mSprites.emplace_back( glm::vec2(), Spr, Phase );
        }
    }
    auto posIterator = outerBorderPositions.begin();
    for ( auto const& borderId : outerBorderIds )
    {
        auto const& id = IdleActionRenderer::GetSpriteId( borderC->GetSpriteIndex(), borderId );
        SpriteCollection const& Sprites = mRenderableRepo( id );
        Sprite const& Spr = Sprites( mActionId );
        if( Spr.IsValid() )
        {
            glm::vec2 pos = *posIterator;
            SpritePhase const& Phase = Spr( ( int32_t )GetState() );
            mSprites.emplace_back( glm::vec2( 2 * pos.x * mActorSize, 2 * pos.y * mActorSize ), Spr, Phase );
        }
        ++posIterator;
    }
    mActorColor = GetColor( actor );
    mRenderableSprites.clear();
    Opt<IRenderableComponent> renderableC = actor.Get<IRenderableComponent>();
    for( auto const& data : mSprites )
    {
        mRenderableSprites.emplace_back( &actor, renderableC.Get(), mActionId, &data.Spr, &data.Phase, mActorColor );
        mRenderableSprites.back().RelativePosition = data.RelativePosition;
    }
}


void BorderActionRenderer::FillRenderableSprites( const Actor& actor, IRenderableComponent const& renderableC, RenderableSprites_t& renderableSprites )
{
    if ( actor.Get<IBorderComponent>()->IsChanged())
    {
        Init( actor );
    }
    renderableSprites.insert( renderableSprites.end(), mRenderableSprites.begin(), mRenderableSprites.end() );
}

} // namespace render

