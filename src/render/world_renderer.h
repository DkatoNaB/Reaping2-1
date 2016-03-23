#ifndef INCLUDED_WORLD_RENDERER_H
#define INCLUDED_WORLD_RENDERER_H

#include "platform/i_platform.h"
#include "vao_base.h"

namespace render {
class WorldRenderer
{
    void Init();
    VaoBase mVAO;
    size_t mTexIndex;
    size_t mPosIndex;
    size_t mColorIndex;
public:
    void Draw( double dt, GLuint solidObjectsTexture, GLuint particlesTexture );
    WorldRenderer();
    ~WorldRenderer();
};
}

#endif // INCLUDED_WORLD_RENDERER_H
