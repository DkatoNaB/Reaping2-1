#include "particle_engine.h"
#include "counter.h"
#include "shader_manager.h"
#include "vao_base.h"
#include "font.h"
#include "i_render.h"
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/ref.hpp>


namespace render {
namespace {
struct Particle
{
    int32_t TexId;
    glm::vec4 TexCoords;
    glm::vec4 Color;
    glm::vec2 Pos;
    glm::vec2 Speed;
    glm::vec2 Acceleration;
    float Heading;
    float RotationSpeed;
    float RotationAcceleration;
    float Lifetime;
    float Radius;
};
typedef std::vector<Particle> Particles;
typedef std::vector<glm::vec2> Vec2s_t;
typedef std::vector<glm::vec4> Vec4s_t;
typedef std::vector<GLfloat> Floats_t;
bool getNextTextId( Particles::const_iterator& i, Particles::const_iterator e,
        Vec2s_t& Positions, Vec4s_t& Colors, Vec4s_t& TexCoords,
        Floats_t& Headings, Floats_t& Radii, Floats_t& Lifetimes,
        GLuint& TexId )
{
    // skip dead particles
    while( i != e && i->Lifetime < 0.0f )
    {
        ++i;
    }
    if( i == e )
    {
        return false;
    }
    Particle const& p = *i;
    Positions.push_back( p.Pos );
    TexCoords.push_back( p.TexCoords );
    Colors.push_back( p.Color );
    Headings.push_back( p.Heading );
    Radii.push_back( p.Radius );
    Lifetimes.push_back( p.Lifetime );
    TexId = p.TexId;
    ++i;
    return true;
}

}
struct ParticleEngineImpl
{
    Particles mParticles;
    float mCycle;
    VaoBase mVAO;
    mutable size_t mPrevParticlesSize;
    ParticleEngineImpl();
    void Update( float dt );
    void Draw() const;
    void AddParticle( int32_t type, glm::vec2 const& pos );
};

ParticleEngineImpl::ParticleEngineImpl()
    : mCycle( 0.0f )
    , mPrevParticlesSize( 0 )
{
    mVAO.Init();
    ShaderManager& ShaderMgr( ShaderManager::Get() );
    ShaderMgr.ActivateShader( "particle" );
    ShaderMgr.UploadData( "spriteTexture", GLuint( 3 ) );
}

void ParticleEngineImpl::Update( float dt )
{
    mCycle += dt;
    for( Particles::iterator i = mParticles.begin(), e = mParticles.end(); i != e; ++i )
    {
        Particle& p = *i;
        p.Lifetime -= dt;
        if( p.Lifetime < 0 )
        {
            continue;
        }
        p.Pos += dt * p.Speed;
        p.Speed += dt * p.Acceleration;
        p.Heading += dt * p.RotationSpeed;
        p.RotationSpeed += dt * p.RotationAcceleration;
    }
    if( mCycle >= 1000. )
    {
        std::remove_if( mParticles.begin(), mParticles.end(), boost::lambda::bind( &Particle::Lifetime, boost::lambda::_1 ) < 0. );
    }
}

/*
//---shader spec
layout(location=0) in vec4 TexCoord;
layout(location=1) in vec2 SpriteCenter;
layout(location=2) in float Heading;
layout(location=3) in vec4 Color;
layout(location=4) in float Radius;
layout(location=5) in float Lifetime;
*/

void ParticleEngineImpl::Draw() const
{
    size_t CurSize = mParticles.size();
    if (CurSize==0)
    {
        return;
    }

    Vec2s_t Positions;
    Vec4s_t Colors;
    Vec4s_t TexCoords;
    Floats_t Headings;
    Floats_t Radii;
    Floats_t Lifetimes;
    Positions.reserve( CurSize );
    Colors.reserve( CurSize );
    TexCoords.reserve( CurSize );
    Headings.reserve( CurSize );
    Radii.reserve( CurSize );
    Lifetimes.reserve( CurSize );

    Particles::const_iterator i = mParticles.begin();
    render::Counts_t const& Counts = render::count(
            boost::lambda::bind( &getNextTextId, boost::ref( i ), mParticles.end(),
                boost::ref( Positions ), boost::ref( Colors ), boost::ref( TexCoords ),
                boost::ref( Headings ), boost::ref( Radii ), boost::ref( Lifetimes ),
                boost::lambda::_1 )
            );

    CurSize = Positions.size();
    if( CurSize == 0 )
    {
        return;
    }

    mVAO.Bind();

    if( CurSize != mPrevParticlesSize )
    {
        size_t TotalSize = CurSize * ( 2 * sizeof( glm::vec4 ) + sizeof( glm::vec2 ) + 3 * sizeof( GLfloat ) );
        glBufferData( GL_ARRAY_BUFFER, TotalSize, NULL, GL_DYNAMIC_DRAW );
        mPrevParticlesSize = CurSize;
    }

    size_t CurrentOffset = 0;
    size_t CurrentSize = 0;
    GLuint CurrentAttribIndex = 0;

    CurrentSize = CurSize * sizeof( glm::vec4 );
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

    CurrentSize = CurSize * sizeof( GLfloat );
    glBufferSubData( GL_ARRAY_BUFFER, CurrentOffset, CurrentSize, &Headings[0] );
    glEnableVertexAttribArray( CurrentAttribIndex );
    size_t const HeadingIndex = CurrentOffset;
    ++CurrentAttribIndex;
    CurrentOffset += CurrentSize;

    CurrentSize = CurSize * sizeof( glm::vec4 );
    glBufferSubData( GL_ARRAY_BUFFER, CurrentOffset, CurrentSize, &Colors[0] );
    glEnableVertexAttribArray( CurrentAttribIndex );
    size_t const ColorIndex = CurrentOffset;
    ++CurrentAttribIndex;
    CurrentOffset += CurrentSize;

    CurrentSize = CurSize * sizeof( GLfloat );
    glBufferSubData( GL_ARRAY_BUFFER, CurrentOffset, CurrentSize, &Radii[0] );
    glEnableVertexAttribArray( CurrentAttribIndex );
    size_t const RadiusIndex = CurrentOffset;
    ++CurrentAttribIndex;
    CurrentOffset += CurrentSize;

    CurrentSize = CurSize * sizeof( GLfloat );
    glBufferSubData( GL_ARRAY_BUFFER, CurrentOffset, CurrentSize, &Lifetimes[0] );
    glEnableVertexAttribArray( CurrentAttribIndex );
    size_t const LifeIndex = CurrentOffset;
    ++CurrentAttribIndex;
    CurrentOffset += CurrentSize;

    ShaderManager& ShaderMgr( ShaderManager::Get() );
    ShaderMgr.ActivateShader( "particle" );
    glActiveTexture( GL_TEXTURE0 + 3 );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE );
    glDepthMask( GL_FALSE );

    for( render::Counts_t::const_iterator i = Counts.begin(), e = Counts.end(); i != e; ++i )
    {
        render::CountByTexId const& Part = *i;
        CurrentAttribIndex = 0;
        glVertexAttribPointer( CurrentAttribIndex, 4, GL_FLOAT, GL_FALSE, 0, ( GLvoid* )( TexIndex + sizeof( glm::vec4 )*Part.Start ) );
        glVertexAttribDivisor( CurrentAttribIndex, 1 );
        ++CurrentAttribIndex;
        glVertexAttribPointer( CurrentAttribIndex, 2, GL_FLOAT, GL_FALSE, 0, ( GLvoid* )( PosIndex + sizeof( glm::vec2 )*Part.Start ) );
        glVertexAttribDivisor( CurrentAttribIndex, 1 );
        ++CurrentAttribIndex;
        glVertexAttribPointer( CurrentAttribIndex, 1, GL_FLOAT, GL_FALSE, 0, ( GLvoid* )( HeadingIndex + sizeof( GLfloat )*Part.Start ) );
        glVertexAttribDivisor( CurrentAttribIndex, 1 );
        ++CurrentAttribIndex;
        glVertexAttribPointer( CurrentAttribIndex, 4, GL_FLOAT, GL_FALSE, 0, ( GLvoid* )( ColorIndex + sizeof( glm::vec4 )*Part.Start ) );
        glVertexAttribDivisor( CurrentAttribIndex, 1 );
        ++CurrentAttribIndex;
        glVertexAttribPointer( CurrentAttribIndex, 1, GL_FLOAT, GL_FALSE, 0, ( GLvoid* )( RadiusIndex + sizeof( GLfloat )*Part.Start ) );
        glVertexAttribDivisor( CurrentAttribIndex, 1 );
        ++CurrentAttribIndex;
        glVertexAttribPointer( CurrentAttribIndex, 1, GL_FLOAT, GL_FALSE, 0, ( GLvoid* )( LifeIndex + sizeof( GLfloat )*Part.Start ) );
        glVertexAttribDivisor( CurrentAttribIndex, 1 );
        if( Part.TexId != GLuint( -1 ) )
        {
            glBindTexture( GL_TEXTURE_2D, Part.TexId );
        }
        glDrawArraysInstanced( GL_TRIANGLE_STRIP, 0, 4, Part.Count );
    }
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glDepthMask( GL_TRUE );
    mVAO.Unbind();
}

void ParticleEngineImpl::AddParticle( int32_t type, glm::vec2 const& pos )
{
    Particle p;
    p.Color = glm::vec4( ( rand() % 256 ) / 512. + 0.5, ( rand() % 256 ) / 512., 0, ( rand() % 256 ) / 512. );
    p.Pos = pos;
    static Font& Fnt( Font::Get() );
    SpritePhase const& Phase = Fnt.GetChar( 'a' + rand() % 27 );
    p.TexId = Phase.TexId;
    p.TexCoords = glm::vec4( Phase.Left, Phase.Right, Phase.Bottom, Phase.Top );
    p.Lifetime = 10;
    p.Heading = 0;
    p.RotationSpeed = 0.1;
    p.Speed = glm::vec2( rand() % 15 - 7, rand() % 15 - 7 );
    p.Acceleration = glm::vec2( 0, 0 );
    p.RotationAcceleration = 0;
    p.Radius = 15 + rand() % 15;
    mParticles.push_back( p );
}

ParticleEngine::ParticleEngine()
    : mImpl( new ParticleEngineImpl() )
{
}

ParticleEngine::~ParticleEngine()
{
}

void ParticleEngine::Update( float dt )
{
    mImpl->Update( dt );
}

void ParticleEngine::Draw() const
{
    mImpl->Draw();
}

void ParticleEngine::AddParticle( int32_t type, glm::vec2 const& pos )
{
    for( int i = 0; i < 10; ++ i)
    {
        mImpl->AddParticle( type, pos );
    }
}

} // namespace render

