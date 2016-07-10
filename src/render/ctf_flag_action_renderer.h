#ifndef INCLUDED_RENDER_CTF_FLAG_ACTION_RENDERER_H
#define INCLUDED_RENDER_CTF_FLAG_ACTION_RENDERER_H

#include "platform/i_platform.h"
#include "render/action_renderer.h"
#include "core/actor.h"
#include "renderable_sprite.h"
#include "renderable_repo.h"
#include "hat_action_renderer.h"

namespace render {
namespace ctf {

class CtfFlagActionRenderer : public ActionRenderer
{
    int32_t mCtfFlagId;
    ColorRepo& mColorRepo;
public:
    CtfFlagActionRenderer( int32_t Id );
    virtual void Init( const Actor& actor );
    virtual void FillRenderableSprites( const Actor& actor, IRenderableComponent const& renderableC, RenderableSprites_t& renderableSprites );
private:
};

} // namespace ctf
} // namespace render

#endif//INCLUDED_RENDER_CTF_FLAG_ACTION_RENDERER_H


//command:  "classgenerator.exe" -g "action_renderer" -c "ctf_flag_action_renderer"
