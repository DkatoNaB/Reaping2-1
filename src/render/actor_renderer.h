#ifndef INCLUDED_RENDER_ACTOR_RENDERER_H
#define INCLUDED_RENDER_ACTOR_RENDERER_H
#include "platform/i_platform.h"
#include "core/scene.h"
#include "recognizer_repo.h"
#include "action_renderer.h"
#include "action_renderer_factory.h"
#include "renderable_sprite.h"
#include "core/actor_event.h"
#include "vao_base.h"
#include "input/mouse.h"
#include "counter.h"
#include "camera.h"
using render::RenderableSprite;
using render::RecognizerRepo;
using render::ActionRenderer;
using render::ActionRendererFactory;
class ActorRenderer
{
public:
    typedef ActionRenderer::RenderableSprites_t RenderableSprites_t;
    typedef boost::function<bool( IRenderableComponent const& )> RenderFilter;
private:
    struct RenderableSpriteCompare
    {
        bool operator()( RenderableSprite const& Rs1, RenderableSprite const& Rs2 );
    };

    void Init();
    RecognizerRepo& mRecognizerRepo;
    ActionRendererFactory& mActionRendererFactory;
    AutoReg mOnActorEvent;
    void OnActorEvent( ActorEvent const& Evt );

    struct FindActionRenderer
    {
        int32_t mActionRendererId;
        FindActionRenderer( int32_t actionRendererId )
            : mActionRendererId( actionRendererId )
        {
        }
        bool operator()( const ActionRenderer& actionRenderer )
        {
            return actionRenderer.GetId() == mActionRendererId;
        }
    };
    typedef boost::ptr_set<ActionRenderer> ActionRenderers_t;
    typedef std::map<int, ActionRenderers_t> ActionRenderersMap_t;
    ActionRenderersMap_t mActionRenderersMap;
    void OnMouseMoveEvent( const WorldMouseMoveEvent& Event );
    AutoReg mMouseMoveId;
    double mX;
    double mY;
    struct RenderDesc
    {
        VaoBase mVAO;
        size_t mPrevSize = 0;
        size_t mTexIndex = 0;
        size_t mPosIndex = 0;
        size_t mHeadingIndex = 0;
        size_t mSizeIndex = 0;
        size_t mColorIndex = 0;
        render::Counts_t mCounts;
        RenderableSprites_t mRenderableSprites;
    };
    RenderDesc mDynamicSprites;
    RenderDesc mStaticSprites;  // TODO take care of dyn. textures
    int32_t mMaxStaticSpriteUID;

    void Prepare( Scene const& scene, Camera const& camera, double deltaTime, RenderDesc& rd );
    void Draw( RenderFilter filter, RenderDesc& rd );
public:
    ActorRenderer();
    ~ActorRenderer();
    void Prepare( Scene const& scene, Camera const& camera, double deltaTime );
    void Draw( RenderFilter filter );

};

#endif//INCLUDED_RENDER_ACTOR_RENDERER_H
