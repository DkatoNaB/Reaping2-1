#ifndef INCLUDED_RENDER_TARGET_H
#define INCLUDED_RENDER_TARGET_H

#include "platform/i_platform.h"

namespace render {
class RenderTarget : public platform::Singleton<RenderTarget>
{
    friend class platform::Singleton<RenderTarget>;
    RenderTarget();
    struct TargetTexture
    {
        // parallel vectors
        std::vector<GLenum> Attachments;
        std::vector<GLuint> TexIds;
        GLuint FramebufferId;
        GLuint DepthBufferId;
        glm::vec2 Size;
    };
    typedef std::map<uint32_t, TargetTexture> TargetMap_t;
    TargetMap_t mTargets;
    mutable uint32_t mCurrentId;
public:
    static uint32_t const ScreenId = uint32_t( -1 );
    uint32_t GetFreeId() const;
    glm::vec2 GetMaxTextureSize() const;
    void SetTargetTexture( uint32_t id, glm::vec2 const& size, size_t numExtraAttachments = 0 );
    void SelectTargetTexture( uint32_t id ) const;
    void SetTargetScreen() const;
    uint32_t GetCurrentTarget() const;
    GLuint GetTextureId( uint32_t id, size_t attachment = 0 );
};
}

#endif

