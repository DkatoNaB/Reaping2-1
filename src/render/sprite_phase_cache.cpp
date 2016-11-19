#include "sprite_phase_cache.h"
#include "render_target.h"
#include "shader_manager.h"
#include "platform/log.h"

namespace render {

SpritePhaseCache::SpritePhaseCache()
{
    static RenderTarget& rt( RenderTarget::Get() );
    mTarget = rt.GetFreeId();
    uint32_t current = rt.GetCurrentTarget();
    mTargetSize = rt.GetMaxTextureSize();
//    mTargetSize = glm::vec2(512,512);
    mRowSize = std::floor( mTargetSize.x / mMaxCellSize );
    L1( "Texture size: %d x %d\n", (int)mTargetSize.x, (int)mTargetSize.y );
    rt.SetTargetTexture( mTarget, mTargetSize );
    rt.SelectTargetTexture( current );
    mTargetTexId = rt.GetTextureId( mTarget );
    mVAO.Init();
    mVAO.Bind();
    size_t TotalSize = 4 * sizeof( glm::vec2 );
    glBufferData( GL_ARRAY_BUFFER, 2 * TotalSize, NULL, GL_DYNAMIC_DRAW );
    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, 0 );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, ( void* )( TotalSize ) );
    mVAO.Unbind();
}

glm::vec4 SpritePhaseCache::FindFreeRegion( SpritePhase const& sprphase )
{
    if( mCacheIndex >= mRowSize * mRowSize )
    {
        return glm::vec4();
    }


    glm::vec4 rv(
            mCacheIndex / mRowSize * mMaxCellSize / mTargetSize.x,
            mCacheIndex % mRowSize * mMaxCellSize / mTargetSize.y,
            ( mCacheIndex / mRowSize + 1 ) * mMaxCellSize / mTargetSize.x,
            ( mCacheIndex % mRowSize + 1 ) * mMaxCellSize / mTargetSize.y
        );
    return rv;
}

void SpritePhaseCache::Draw( SpritePhase const& sprphase, glm::vec4 const& freeRegion )
{
    std::vector<glm::vec2> Positions;
    std::vector<glm::vec2> TexCoords;

    L1( "region: %f %f %f %f\n",
            freeRegion.x,
            freeRegion.y,
            freeRegion.z,
            freeRegion.w );

    Positions.emplace_back( freeRegion.x, freeRegion.y );
    Positions.emplace_back( freeRegion.x, freeRegion.w );
    Positions.emplace_back( freeRegion.z, freeRegion.y );
    Positions.emplace_back( freeRegion.z, freeRegion.w );

/*    Positions.emplace_back( -0.5, -0.5 );
    Positions.emplace_back( -0.5, 0.5 );
    Positions.emplace_back( 0.5, -0.5 );
    Positions.emplace_back( 0.5, 0.5 ); */

    TexCoords.emplace_back( sprphase.Left, sprphase.Bottom );
    TexCoords.emplace_back( sprphase.Left, sprphase.Top );
    TexCoords.emplace_back( sprphase.Right, sprphase.Bottom );
    TexCoords.emplace_back( sprphase.Right, sprphase.Top );

    size_t CurrentOffset = 0;
    size_t CurrentSize = 0;
    GLuint CurrentAttribIndex = 0;
    size_t CurSize = Positions.size();

    CurrentSize = CurSize * sizeof( glm::vec2 );
    glBufferSubData( GL_ARRAY_BUFFER, CurrentOffset, CurrentSize, &TexCoords[0] );
    size_t const TexIndex = CurrentOffset;
    CurrentOffset += CurrentSize;

    CurrentSize = CurSize * sizeof( glm::vec2 );
    glBufferSubData( GL_ARRAY_BUFFER, CurrentOffset, CurrentSize, &Positions[0] );
    size_t const PosIndex = CurrentOffset;

    glBindTexture( GL_TEXTURE_2D, sprphase.TexId );
    glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
}

void SpritePhaseCache::Request( SpritePhase const& sprphase )
{
    if( sprphase.TexId == mTargetTexId )
    {
        return;
    }
    if( false && sprphase.Right - sprphase.Left > 200 )
    {
        return;
    }
    glm::vec4 freeRegion = FindFreeRegion( sprphase );
    if( freeRegion.x == 0 &&
        freeRegion.z == 0 &&
        freeRegion.y == 0 &&
        freeRegion.w == 0 )
    {
        return;
    }
    mPending.insert( &sprphase );
}

// simply render to a 200*200 grid, or 100*100 + 200*200
// use (up)scaling
// intel supports a 8k*8k texture, which results in 1.6k cached spritephases
// that oughta be enough
void SpritePhaseCache::ProcessPending()
{
    if( mPending.empty() )
    {
        return;
    }
    static RenderTarget& rt( RenderTarget::Get() );
    rt.SelectTargetTexture( mTarget );
    glEnable( GL_TEXTURE_2D );
//    glBlendFunc( GL_ONE, GL_ZERO );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glViewport( 0, 0, mTargetSize.x, mTargetSize.y );
    ShaderManager& ShaderMgr( ShaderManager::Get() );
    ShaderMgr.ActivateShader( "combiner" );
    ShaderMgr.UploadData( "texture", GLuint( 1 ) );
    glActiveTexture( GL_TEXTURE0 + 1 );
    mVAO.Bind();

    for( auto const* spr : mPending )
    {
        auto const& sprphase = *spr;
        mOriginal[ spr ] = sprphase;

        // copy the sprite to the temp map
        auto const& freeRegion = FindFreeRegion( sprphase );
        if( freeRegion.x == 0 &&
            freeRegion.z == 0 &&
            freeRegion.y == 0 &&
            freeRegion.w == 0 )
        {
            break;
        }
        ++mCacheIndex;

        Draw( sprphase, freeRegion );

        SpritePhase& mutablePhase = const_cast<SpritePhase&>( sprphase );

        L1( "spr: %d %f %f %f %f\n",
                mutablePhase.TexId,
                mutablePhase.Left,
                mutablePhase.Bottom,
                mutablePhase.Right,
                mutablePhase.Top );

        mutablePhase.Left = freeRegion.x;
        mutablePhase.Bottom = freeRegion.y;
        mutablePhase.Right = freeRegion.z;
        mutablePhase.Top = freeRegion.w;

        mutablePhase.TexId = mTargetTexId;
        L1( "spr post: %d %f %f %f %f\n",
                mutablePhase.TexId,
                mutablePhase.Left,
                mutablePhase.Bottom,
                mutablePhase.Right,
                mutablePhase.Top );

    }
    mVAO.Unbind();
    glActiveTexture( GL_TEXTURE0 );
    mPending.clear();
}
}


