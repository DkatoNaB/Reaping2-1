#include "i_render.h"
#include "actor_renderer.h"
#include "core/i_position_component.h"
#include "core/i_inventory_component.h"
#include "core/i_health_component.h"
#include "core/i_collision_component.h"
#include "core/i_renderable_component.h"
#include "core/actor.h"
#include "recognizer.h"
#include "shader_manager.h"
#include "input/mouse.h"
#include "engine/engine.h"
#include "core/program_state.h"
#include "core/scene.h"
#include "renderable_repo.h"
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/ref.hpp>

void ActorRenderer::Init()
{
    mDynamicSprites.mVAO.Init();
    mStaticSprites.mVAO.Init();
    mOnActorEvent = EventServer<ActorEvent>::Get().Subscribe( boost::bind( &ActorRenderer::OnActorEvent, this, _1 ) );
    mMouseMoveId = EventServer<WorldMouseMoveEvent>::Get().Subscribe( boost::bind( &ActorRenderer::OnMouseMoveEvent, this, _1 ) );
}

void ActorRenderer::OnActorEvent( ActorEvent const& Evt )
{
    if( Evt.mState == ActorEvent::Added )
    {
        mActionRenderersMap[Evt.mActor->GetGUID()];
    }
    else if ( Evt.mState == ActorEvent::Removed )
    {
        mActionRenderersMap.erase( Evt.mActor->GetGUID() );
    }
}

ActorRenderer::ActorRenderer()
    : mRecognizerRepo( RecognizerRepo::Get() )
    , mActionRendererFactory( ActionRendererFactory::Get() )
    , mX(0)
    , mY(0)
    , mMaxStaticSpriteUID(0)
{
    Init();
}

namespace {
typedef std::vector<glm::vec2> Positions_t;
typedef std::vector<GLfloat> Floats_t;
typedef std::vector<glm::vec4> TexCoords_t;
typedef std::vector<glm::vec4> Colors_t;
typedef ActorRenderer::RenderableSprites_t RenderableSprites_t;
bool isVisible( Actor const& actor, glm::vec4 const& region )
{
    Opt<IPositionComponent> const positionC = actor.Get<IPositionComponent>();
    if( !positionC.IsValid() )
    {
        return false;
    }
    static std::map<int32_t, float> scaleMap;
    auto it = scaleMap.find( actor.GetId() );
    if( scaleMap.end() == it )
    {
        float& f = scaleMap[ actor.GetId() ];
        static RenderableRepo& renderables( RenderableRepo::Get() );
        SpriteCollection const& Sprites = renderables( actor.GetId() );
        for( auto i = Sprites.begin(), e = Sprites.end(); i != e; ++i )
        {
            if( i->second->GetScale() > f )
            {
                f = i->second->GetScale();
            }
        }
        it = scaleMap.find( actor.GetId() );
    }
    float scale = it->second;
    Opt<ICollisionComponent> const collisionC = actor.Get<ICollisionComponent>();
    // 2.0 multiplier: safety
    float size = ( collisionC.IsValid() ? collisionC->GetRadius() : 50 ) * scale * 2.0;
    return region.x < positionC->GetX() + size && region.z > positionC->GetX() - size
        && region.y < positionC->GetY() + size && region.w > positionC->GetY() - size;
}
bool getNextTextId( RenderableSprites_t::const_iterator& i, RenderableSprites_t::const_iterator e,
                    glm::vec2*& Positions, GLfloat*& Headings, glm::vec4*& TexCoords, GLfloat*& Sizes, glm::vec4*& Colors,
                    GLuint& TexId )
{
    if( i == e )
    {
        return false;
    }
    TexId = i->Spr->TexId;
    (*Positions++) = glm::vec2( i->PositionC->GetX(), i->PositionC->GetY() ) + i->RelativePosition;
    (*Headings++) = ( GLfloat )i->PositionC->GetOrientation();

    (*Sizes++) = ( GLfloat )( ( i->CollisionC != nullptr ? i->CollisionC->GetRadius() : 50 )*i->Anim->GetScale() );
    (*TexCoords++) = glm::vec4( i->Spr->Left, i->Spr->Right, i->Spr->Bottom, i->Spr->Top );
    (*Colors++) = i->Color;
    ++i;
    return true;
}
}

void ActorRenderer::Prepare( Scene const& Object, Camera const& camera, double DeltaTime )
{
    static int st = 0;
    if( (++st % 20) == 0 )
    {
        Prepare( Object, camera, DeltaTime, mStaticSprites );
    }
    Prepare( Object, camera, DeltaTime, mDynamicSprites );
}

void ActorRenderer::Prepare( Scene const& Object, Camera const& camera, double DeltaTime, RenderDesc& rd )
{
    bool const dyn = &rd == &mDynamicSprites;
    ActorList_t const& Lst = Object.GetActors();
    if( Lst.empty() )
    {
        rd.mRenderableSprites.clear(); // renderable sprites still can contain obsolete sprites, so render nothing instead of invalid object
        rd.mCounts.clear();
        return;
    }
    glm::vec4 region = camera.VisibleRegion();
    if( !dyn )
    {
        if( mStaticRegion.x <= region.x && mStaticRegion.z >= region.z
                && mStaticRegion.y <= region.y && mStaticRegion.w >= region.w )
        {
            return;
        }
        float w = region.z - region.x;
        float h = region.w - region.y;
//        region += glm::vec4( -w, -h, w, h ) / 2.0f;
        mStaticRegion = region;
        mMaxStaticSpriteUID = 0;
    }
    RenderableSprites_t RenderableSprites;
    RenderableSprites.reserve( rd.mRenderableSprites.size() );
    //the template version works well with '=' i just dont know is it really needed, maybe this one is more self explaining
    ActorListFilter<Scene::RenderableActors> wrp( Lst ); //=Object.GetActors<Scene::RenderableComponents>();
    int32_t maxGUID = 0;
    for( ActorListFilter<Scene::RenderableActors>::const_iterator i = wrp.begin(), e = wrp.end(); i != e; ++i )
    {
        const Actor& Object = **i;
        if( !isVisible( Object, region ) )
        {
            continue;
        }
        bool const hasHealth = Object.Get<IHealthComponent>().IsValid();
        if( dyn && Object.GetGUID() <= mMaxStaticSpriteUID && !hasHealth )
        {
            continue;
        }
        if( !dyn && hasHealth )
        {
            continue;
        }
        auto recogptr = mRecognizerRepo.GetRecognizers( Object.GetId() );
        if( nullptr == recogptr )
        {
            continue;
        }
        if( maxGUID < Object.GetGUID() )
        {
            maxGUID = Object.GetGUID();
        }
        Opt<IRenderableComponent> renderableC( Object.Get<IRenderableComponent>() );
        auto const& recognizers = *recogptr;
        RecognizerRepo::ExcludedRecognizers_t excluded;

        ActionRenderersMap_t::iterator actionRenderersIt = mActionRenderersMap.find( Object.GetGUID() );
        BOOST_ASSERT( actionRenderersIt != mActionRenderersMap.end() );
        ActionRenderers_t& actionRenderers = actionRenderersIt->second;

        for ( auto recogIt = recognizers.begin(), recogE = recognizers.end(); recogIt != recogE; ++recogIt )
        {
            auto const& recognizer = *recogIt;
            if ( excluded.find( recognizer.GetId() ) == excluded.end()
                 && recognizer.Recognize( Object ) )
            {
                int32_t actionRendererId = recognizer.GetActionRenderer();
                ActionRenderers_t::iterator foundActionRendererIt =
                    std::find_if( actionRenderers.begin(), actionRenderers.end(), FindActionRenderer( actionRendererId ) );
                if ( foundActionRendererIt == actionRenderers.end() )
                {
                    std::auto_ptr<ActionRenderer> actionRenderer( mActionRendererFactory( actionRendererId ) );
                    actionRenderer->SetOrder( recognizer.GetOrder() );
                    actionRenderer->Init( Object );
                    actionRenderers.insert( actionRenderer );
                }
                auto excludedRecognizers = mRecognizerRepo.GetExcludedRecognizers( recognizer.GetId() );
                if( nullptr != excludedRecognizers )
                {
                    excluded.insert( excludedRecognizers->begin(), excludedRecognizers->end() );
                }
            }
        }
        for ( auto excludedIt = excluded.begin(), excludedE = excluded.end(); excludedIt != excludedE; ++excludedIt )
        {
            auto arIt = std::find_if( actionRenderers.begin(), actionRenderers.end(), FindActionRenderer( *excludedIt ) );
            if( arIt != actionRenderers.end() )
            {
                actionRenderers.erase( arIt );

            }
        }
        for ( auto actionRendererIt = actionRenderers.begin(), actionRendererE = actionRenderers.end(); actionRendererIt != actionRendererE; ++actionRendererIt )
        {
            ActionRenderer& actionRenderer = *actionRendererIt;
            actionRenderer.FillRenderableSprites( Object, *renderableC.Get(), RenderableSprites );
            actionRenderer.Update( DeltaTime );
        }
    }

    std::swap( RenderableSprites, rd.mRenderableSprites );
    size_t CurSize = rd.mRenderableSprites.size();
    if( CurSize == 0 )
    {
        return;
    }

    Positions_t Positions( CurSize );
    Floats_t Headings( CurSize );
    Floats_t Sizes( CurSize );
    TexCoords_t TexCoords( CurSize );
    Colors_t Colors( CurSize );

    glm::vec2* posptr = &Positions[0];
    GLfloat* hptr = &Headings[0];
    GLfloat* sptr = &Sizes[0];
    glm::vec4* tptr = &TexCoords[0];
    glm::vec4* cptr = &Colors[0];
    RenderableSprites_t::const_iterator i = rd.mRenderableSprites.begin();
    rd.mCounts = render::count(
        boost::lambda::bind( &getNextTextId, boost::ref( i ), rd.mRenderableSprites.end(),
        boost::ref( posptr ), boost::ref( hptr ), boost::ref( tptr ), boost::ref( sptr ), boost::ref( cptr ),
        boost::lambda::_1 )
    );
    rd.mVAO.Bind();

    if( CurSize > rd.mPrevSize )
    {
        rd.mPrevSize = CurSize;
        size_t TotalSize = CurSize * ( sizeof( glm::vec4 ) + sizeof( glm::vec2 ) + 2 * sizeof( GLfloat ) + sizeof( glm::vec4 ) );
        glBufferData( GL_ARRAY_BUFFER, TotalSize, NULL, GL_DYNAMIC_DRAW );
    }

    size_t CurrentOffset = 0;
    size_t CurrentSize = CurSize * sizeof( glm::vec4 );
    GLuint CurrentAttribIndex = 0;
    glBufferSubData( GL_ARRAY_BUFFER, CurrentOffset, CurrentSize, &TexCoords[0] );
    glEnableVertexAttribArray( CurrentAttribIndex );
    rd.mTexIndex = CurrentOffset;
    ++CurrentAttribIndex;

    CurrentOffset += CurrentSize;
    CurrentSize = CurSize * sizeof( glm::vec2 );
    glBufferSubData( GL_ARRAY_BUFFER, CurrentOffset, CurrentSize, &Positions[0] );
    glEnableVertexAttribArray( CurrentAttribIndex );
    rd.mPosIndex = CurrentOffset;
    ++CurrentAttribIndex;

    CurrentOffset += CurrentSize;
    CurrentSize = CurSize * sizeof( GLfloat );
    glBufferSubData( GL_ARRAY_BUFFER, CurrentOffset, CurrentSize, &Headings[0] );
    glEnableVertexAttribArray( CurrentAttribIndex );
    rd.mHeadingIndex = CurrentOffset;
    ++CurrentAttribIndex;

    CurrentOffset += CurrentSize;
    CurrentSize = CurSize * sizeof( GLfloat );
    glBufferSubData( GL_ARRAY_BUFFER, CurrentOffset, CurrentSize, &Sizes[0] );
    glEnableVertexAttribArray( CurrentAttribIndex );
    rd.mSizeIndex = CurrentOffset;
    ++CurrentAttribIndex;

    CurrentOffset += CurrentSize;
    CurrentSize = CurSize * sizeof( glm::vec4 );
    glBufferSubData( GL_ARRAY_BUFFER, CurrentOffset, CurrentSize, &Colors[0] );
    glEnableVertexAttribArray( CurrentAttribIndex );
    rd.mColorIndex = CurrentOffset;
    rd.mVAO.Unbind();

    if( !dyn )
    {
        mMaxStaticSpriteUID = maxGUID;
    }
}

namespace {
void partitionByFilter( render::Counts_t& rv, RenderableSprites_t const& sprites, render::CountByTexId const& part, ActorRenderer::RenderFilter const& filter )
{
    rv.clear();
    size_t idx = part.Start;
    render::CountByTexId* actual = NULL;
    bool match = false;
    IRenderableComponent const* prevRC = nullptr;
    for( auto i = sprites.begin() + part.Start, e = sprites.begin() + part.Start + part.Count; i != e; ++i, ++idx )
    {
        auto const& val = *i;
        if( prevRC != val.RenderableComp )
        {
            match = filter( *val.RenderableComp );
            prevRC = val.RenderableComp;
        }
        if( !match )
        {
            actual = NULL;
        }
        else
        {
            if( NULL == actual )
            {
                rv.push_back( part );
                actual = &rv.back();
                actual->Start = idx;
                actual->Count = 0;
            }
            ++actual->Count;
        }
    }
}
}

void ActorRenderer::Draw( RenderFilter filter )
{
    Draw( filter, mStaticSprites );
    Draw( filter, mDynamicSprites );
}

void ActorRenderer::Draw( RenderFilter filter, RenderDesc& rd )
{
    rd.mVAO.Bind();
    ShaderManager& ShaderMgr( ShaderManager::Get() );
    ShaderMgr.ActivateShader( "sprite2" );
    ShaderMgr.UploadData( "spriteTexture", GLuint( 1 ) );
    glActiveTexture( GL_TEXTURE0 + 1 );
    GLuint CurrentAttribIndex = 0;
    for( render::Counts_t::const_iterator i = rd.mCounts.begin(), e = rd.mCounts.end(); i != e; ++i )
    {
        render::CountByTexId const& BigPart = *i;
        glBindTexture( GL_TEXTURE_2D, BigPart.TexId );
        static render::Counts_t parts;
        partitionByFilter( parts, rd.mRenderableSprites, BigPart, filter );
        for( auto const& Part : parts )
        {
            CurrentAttribIndex = 0;
            glVertexAttribPointer( CurrentAttribIndex, 4, GL_FLOAT, GL_FALSE, 0, ( GLvoid* )( rd.mTexIndex + sizeof( glm::vec4 )*Part.Start ) );
            glVertexAttribDivisor( CurrentAttribIndex, 1 );
            ++CurrentAttribIndex;
            glVertexAttribPointer( CurrentAttribIndex, 2, GL_FLOAT, GL_FALSE, 0, ( GLvoid* )( rd.mPosIndex + sizeof( glm::vec2 )*Part.Start ) );
            glVertexAttribDivisor( CurrentAttribIndex, 1 );
            ++CurrentAttribIndex;
            glVertexAttribPointer( CurrentAttribIndex, 1, GL_FLOAT, GL_FALSE, 0, ( GLvoid* )( rd.mHeadingIndex + sizeof( GLfloat )*Part.Start ) );
            glVertexAttribDivisor( CurrentAttribIndex, 1 );
            ++CurrentAttribIndex;
            glVertexAttribPointer( CurrentAttribIndex, 1, GL_FLOAT, GL_FALSE, 0, ( GLvoid* )( rd.mSizeIndex + sizeof( GLfloat )*Part.Start ) );
            glVertexAttribDivisor( CurrentAttribIndex, 1 );
            ++CurrentAttribIndex;
            glVertexAttribPointer( CurrentAttribIndex, 4, GL_FLOAT, GL_FALSE, 0, ( GLvoid* )( rd.mColorIndex + sizeof( glm::vec4 )*Part.Start ) );
            glVertexAttribDivisor( CurrentAttribIndex, 1 );
            glDrawArraysInstanced( GL_TRIANGLE_STRIP, 0, 4, Part.Count );
        }
    }
    glActiveTexture( GL_TEXTURE0 );
    rd.mVAO.Unbind();
}

ActorRenderer::~ActorRenderer()
{

}

void ActorRenderer::OnMouseMoveEvent( const WorldMouseMoveEvent& Event )
{
    mX = Event.Pos.x;
    mY = Event.Pos.y;
}

bool ActorRenderer::RenderableSpriteCompare::operator()( RenderableSprite const& Rs1, RenderableSprite const& Rs2 )
{
    Opt<IRenderableComponent> Rs1RenderableC = Rs1.Obj->Get<IRenderableComponent>();
    Opt<IRenderableComponent> Rs2RenderableC = Rs2.Obj->Get<IRenderableComponent>();
    return Rs1RenderableC->GetLayer() < Rs2RenderableC->GetLayer() ||
           ( Rs1RenderableC->GetLayer() == Rs2RenderableC->GetLayer() &&
             ( Rs1RenderableC->GetZOrder() < Rs2RenderableC->GetZOrder() ||
               ( Rs1RenderableC->GetZOrder() == Rs2RenderableC->GetZOrder() &&
                 ( Rs1.ActId < Rs2.ActId ||
                   ( Rs1.ActId == Rs2.ActId &&
                     Rs1.Spr->TexId < Rs2.Spr->TexId ) ) ) ) );
}
