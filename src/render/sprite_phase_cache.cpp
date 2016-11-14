#include "sprite_phase_cache.h"
#include "render_target.h"
#include "platform/log.h"

namespace render {

SpritePhaseCache::SpritePhaseCache()
{
    static RenderTarget& rt( RenderTarget::Get() );
    mTarget = rt.GetFreeId();
    uint32_t current = rt.GetCurrentTarget();
    mTargetSize = rt.GetMaxTextureSize();
    L1( "Texture size: %d x %d\n", (int)mTargetSize.x, (int)mTargetSize.y );
    rt.SetTargetTexture( mTarget, mTargetSize );
    rt.SelectTargetTexture( current );
    mTargetTexId = rt.GetTextureId( mTarget );
}

glm::vec4 SpritePhaseCache::FindFreeRegion( SpritePhase const& sprphase )
{
    return glm::vec4( sprphase.Left, sprphase.Top, sprphase.Right, sprphase.Bottom );
}

void SpritePhaseCache::Request( SpritePhase const& sprphase )
{
    if( sprphase.TexId == mTargetTexId )
    {
        return;
    }
    if( sprphase.Right - sprphase.Left > 200 )
    {
        return;
    }
    glm::vec4 freeRegion = FindFreeRegion( sprphase );
    if( freeRegion.x == freeRegion.z )
    {
        return;
    }

    mOriginal[ &sprphase ] = sprphase;
    // copy the sprite to the temp map
    GLuint olderr = glGetError();

    // GODDARNIT. gotta get the original dimensions, or use "normal" rendering
    // normal rendering seems better.
    // note: maybe have a collecting and a rendering phase?

    glCopyImageSubData( sprphase.TexId, GL_TEXTURE_2D, 0, sprphase.Left, sprphase.Top, 0,
                        mTargetTexId,   GL_TEXTURE_2D, 0, freeRegion.x, freeRegion.y, 0,
                                                  sprphase.Right - sprphase.Left,
                                                  sprphase.Bottom - sprphase.Top,11);
    GLuint newerr = glGetError();
    L1( "Result: %d %d %f %f %f %f %d %d %d %d\n",
            olderr, newerr,
            freeRegion.x, freeRegion.y, freeRegion.z, freeRegion.w,
            sprphase.Left, sprphase.Top, sprphase.Right, sprphase.Bottom );
    // mutate
    SpritePhase& mutablePhase = const_cast<SpritePhase&>( sprphase );
    mutablePhase.Left = freeRegion.x;
    mutablePhase.Top = freeRegion.y;
    mutablePhase.Right = freeRegion.z;
    mutablePhase.Bottom = freeRegion.w;
    mutablePhase.TexId = mTargetTexId;
}
}


