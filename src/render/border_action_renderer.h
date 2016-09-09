#ifndef INCLUDED_RENDER_BORDER_ACTION_RENDERER_H
#define INCLUDED_RENDER_BORDER_ACTION_RENDERER_H

#include "platform/i_platform.h"
#include "render/action_renderer.h"
#include "core/actor.h"
#include "renderable_sprite.h"
#include "renderable_repo.h"
#include "core/border_type.h"
#include "core/i_border_component.h"

namespace render {

class BorderActionRenderer : public ActionRenderer
{
    int32_t mActionId;
    double mActorSize;
    struct SpriteData {
        glm::vec2 RelativePosition;
        Sprite const& Spr;
        SpritePhase const& Phase;
        SpriteData( glm::vec2 const& rp, Sprite const& sp, SpritePhase const& p ) : RelativePosition( rp ), Spr( sp ), Phase( p ) {}
    };
    std::vector<SpriteData> mSprites;
    glm::vec4 mActorColor;
    RenderableSprites_t mRenderableSprites;
public:
    BorderActionRenderer( int32_t Id );
    virtual void Init( const Actor& actor );
    virtual void FillRenderableSprites( const Actor& actor, IRenderableComponent const& renderableC, RenderableSprites_t& renderableSprites );
private:
};

} // namespace render

#endif//INCLUDED_RENDER_BORDER_ACTION_RENDERER_H

//command:  "classgenerator" -g "action_renderer" -c "border_action_renderer"
