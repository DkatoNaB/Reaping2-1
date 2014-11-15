#include "i_core.h"

const uint32_t Grid::Collisions[] =
{
    0,                                                                                                              // no collision
    1 << CollisionClass::Creep | 1 << CollisionClass::Mine | 1 << CollisionClass::Player | 1 << CollisionClass::Wall, // projectile
    1 << CollisionClass::Projectile | 1 << CollisionClass::Mine | 1 << CollisionClass::Player | 1 << CollisionClass::Wall, // creep
    1 << CollisionClass::Projectile | 1 << CollisionClass::Creep | 1 << CollisionClass::Wall,                       // mine
    1 << CollisionClass::Projectile | 1 << CollisionClass::Creep | 1 << CollisionClass::Player | 1 << CollisionClass::Wall | 1 << CollisionClass::Pickup, // player
    1 << CollisionClass::Projectile | 1 << CollisionClass::Creep | 1 << CollisionClass::Mine | 1 << CollisionClass::Player, // wall
    1 << CollisionClass::Player,                                                                                    // pickup
};

PossibleCollisions_t Grid::GetPossibleCollisions()const
{
    PossibleCollisions_t PossibleCollisions;
    for( Cells_t::const_iterator i = mCells.begin(), e = mCells.end(); i != e; ++i )
    {
        Cell const& C = *i;
        for( size_t k = 0; k < CollisionClass::Num_Classes; ++k )
        {
            Actors_t const& Actors = C.mActors[k];
            for( size_t j = k; j < CollisionClass::Num_Classes; ++j )
            {
                if( !( Collisions[k] & ( 1 << j ) ) )
                {
                    continue;
                }
                Actors_t const& Actors2 = C.mActors[j];
                for( Actors_t::const_iterator a = Actors.begin(), ea = Actors.end(); a != ea; ++a )
                    for( Actors_t::const_iterator b = Actors2.begin(), eb = Actors2.end(); b != eb; ++b )
                    {
                        if( *a == *b )
                        {
                            continue;
                        }
                        PossibleCollisions.insert( CollPair( *a, *b ) );
                    }
            }
        }
    }
    return PossibleCollisions;
}

void Grid::Build( glm::vec4 const& Dimensions, float CellSize )
{
    mCells.clear();
    mCellSize = CellSize;
    mMin = glm::vec2( Dimensions.x, Dimensions.y );
    if( mCellSize < std::numeric_limits<float>::epsilon() )
    {
        mDimX = mDimY = 0;
        assert( false );
        return;
    }
    float const Width = Dimensions.z - Dimensions.x;
    float const Height = Dimensions.w - Dimensions.y;
    mDimX = ( size_t )glm::ceil( Width / mCellSize );
    mDimY = ( size_t )glm::ceil( Height / mCellSize );
    mCells.resize( mDimX * mDimY );
}

void Grid::AddActor( Actor* A, double Dt )
{
    int32_t const CC = A->GetCC();
    if( !Collisions[CC] )
    {
        return;
    }
    size_t const NumCells = mCells.size();
    if( NumCells == 0 )
    {
        assert( false );
        return;
    }
    else if( NumCells == 1 )
    {
        mCells[0].mActors[CC].push_back( A );
        return;
    }
    glm::vec4 const& ActorDim = Box( *A, Dt );

    size_t const Ex = ( size_t )glm::floor( std::max<float>( 0.0f, ActorDim.z ) / mCellSize );
    size_t const Ey = ( size_t )glm::floor( std::max<float>( 0.0f, ActorDim.w ) / mCellSize );
    size_t const Sx = ( size_t )glm::floor( std::max<float>( 0.0f, ActorDim.x ) / mCellSize );
    size_t const Sy = ( size_t )glm::floor( std::max<float>( 0.0f, ActorDim.y ) / mCellSize );
    for( size_t y = Sy, ey = std::min<size_t>( Ey + 1, mDimY ); y < ey; ++y )
        for( size_t x = Sx, ex = std::min<size_t>( Ex + 1, mDimX ); x < ex; ++x )
        {
            mCells[y * mDimX + x].mActors[CC].push_back( A );
        }
}

glm::vec4 Grid::Box( Actor const& Obj, double Dt )const
{
    float const MvX = Dt * Obj.GetSpeedX();
    float const MvY = Dt * Obj.GetSpeedY();
    double const Radius = Obj.GetRadius();
	Opt<PositionComponent> const objPositionC = Obj.Get<PositionComponent>();
    double const Ox = objPositionC->GetX() - mMin.x;
    double const Oy = objPositionC->GetY() - mMin.y;
    glm::vec4 Ret( Ox - Obj.GetRadius(),
                   Oy - Obj.GetRadius(),
                   Ox + Obj.GetRadius(),
                   Oy + Obj.GetRadius() );
    if( MvX < 0.0 )
    {
        Ret.x += MvX;
    }
    else
    {
        Ret.z += MvX;
    }
    if( MvY < 0.0 )
    {
        Ret.y += MvY;
    }
    else
    {
        Ret.w += MvY;
    }
    return Ret;
}

void Grid::Clear()
{
    mCells.assign( mCells.size(), Cell() );
}

CollPair::CollPair( Actor* a1, Actor* a2 ) : A1( a1 < a2 ? a1 : a2 )
    , A2( a1 < a2 ? a2 : a1 )
{
    assert( a1 != a2 );
}

bool CollPairCmp::operator()( CollPair const& a, CollPair const& b ) const
{
    return a.A1 < b.A1 || ( a.A1 == b.A1 && a.A2 < b.A2 );
}
