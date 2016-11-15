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
    mRowSize = std::floor( mTargetSize.x / mMaxCellSize );
    L1( "Texture size: %d x %d\n", (int)mTargetSize.x, (int)mTargetSize.y );
    rt.SetTargetTexture( mTarget, mTargetSize );
    rt.SelectTargetTexture( current );
    mTargetTexId = rt.GetTextureId( mTarget );
    mVAO.Init();
    mVAO.Bind();
    size_t TotalSize = 4 * 2 * sizeof( glm::vec2 );
    glBufferData( GL_ARRAY_BUFFER, TotalSize, NULL, GL_DYNAMIC_DRAW );
    mVAO.Unbind();
}

glm::vec4 SpritePhaseCache::FindFreeRegion( SpritePhase const& sprphase )
{
    if( mCacheIndex >= mRowSize * mRowSize )
    {
        return glm::vec4();
    }
    return glm::vec4
        (
            mCacheIndex / mRowSize * mMaxCellSize
            mCacheIndex % mRowSize * mMaxCellSize
            ( mCacheIndex / mRowSize + 1 ) * mMaxCellSize
            ( mCacheIndex % mRowSize + 1 ) * mMaxCellSize
        );
}

void SpritePhaseCache::Draw( SpritePhase const& sprphase )
{
    std::vector<glm::vec2> Positions;
    std::vector<glm::vec2> TexCoords;
    mVAO.Bind();

    size_t CurrentOffset = 0;
    size_t CurrentSize = 0;
    GLuint CurrentAttribIndex = 0;
    size_t CurSize = Positions.size();

    CurrentSize = CurSize * sizeof( glm::vec2 );
    glBufferSubData( GL_ARRAY_BUFFER, CurrentOffset, CurrentSize, &TexCoords[0] );
    glEnableVertexAttribArray( CurrentAttribIndex );
    size_t const TexIndex = CurrentOffset;
    ++CurrentAttribIndex;
    CurrentOffset += CurrentSize;

    CurrentSize = CurSize * sizeof( glm::vec2 );
    glBufferSubData( GL_ARRAY_BUFFER, CurrentOffset, CurrentSize, &Positions[0] );
    glEnableVertexAttribArray( CurrentAttribIndex );
    size_t const PosIndex = CurrentOffset;
    ++CurrentAttribIndex;
    CurrentOffset += CurrentSize;

        CurrentAttribIndex = 0;
        glVertexAttribPointer( CurrentAttribIndex, 2, GL_FLOAT, GL_FALSE, 0, ( GLvoid* )( TexIndex ) );
        glVertexAttribDivisor( CurrentAttribIndex, 1 );
        ++CurrentAttribIndex;

        glVertexAttribPointer( CurrentAttribIndex, 2, GL_FLOAT, GL_FALSE, 0, ( GLvoid* )( PosIndex ) );
        glVertexAttribDivisor( CurrentAttribIndex, 1 );
        ++CurrentAttribIndex;

    glBindTexture( GL_TEXTURE_2D, Part.TexId );
    glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

    mVAO.Unbind();
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
    mPending.push_back( &sprphase );
}

// simply render to a 200*200 grid, or 100*100 + 200*200
// use (up)scaling
// intel supports a 8k*8k texture, which results in 1.6k cached spritephases
// that oughta be enough
void SpritePhaseCache::ProcessPending()
{
    static RenderTarget& rt( RenderTarget::Get() );
    rt.SelectTargetTexture( mTarget );
    glEnable( GL_TEXTURE_2D );
    glBlendFunc( GL_ONE, GL_ONE );
   //  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable( GL_BLEND );
    glViewport( 0, 0, mTargetSize.x, mTargetSize.y );
    ShaderManager& ShaderMgr( ShaderManager::Get() );
    ShaderMgr.ActivateShader( "combiner" );
    ShaderMgr.UploadData( "texture", GLuint( 1 ) );
    glActiveTexture( GL_TEXTURE0 + 1 );
    mVAO.Bind();



    glBindTexture( GL_TEXTURE_2D, texture );
    glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
    for( auto const* spr : mPending )
    {
        auto const& sprphase = *spr;
        mOriginal[ spr ] = sprphase;
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
    mVAO.Unbind();
    glActiveTexture( GL_TEXTURE0 );
    mPending.clear();
}
}


