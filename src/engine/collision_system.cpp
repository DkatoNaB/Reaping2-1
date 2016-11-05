#include "platform/i_platform.h"
#include "engine/collision_system.h"
#include "core/i_collision_component.h"
#include "boost/assert.hpp"
#include "core/collision_model.h"
#include "core/i_position_component.h"
#include "core/i_move_component.h"
#include <atomic>
#include <thread>
#include <future>
#include <chrono>

namespace engine {

CollisionSystem::CollisionSystem()
    : mCollisionStore( CollisionStore::Get() )
    , mScene( Scene::Get() )
{

}

void CollisionSystem::Init()
{
    SubSystemHolder::Init();
    mCollisionGrid.Build( mScene.GetDimensions(), 400.0f );
    mScene.AddValidator( GetType_static(), []( Actor const& actor )->bool {
        return actor.Get<ICollisionComponent>().IsValid()
            && actor.Get<IPositionComponent>().IsValid()
            && actor.Get<IMoveComponent>().IsValid(); } );
    mOnActorEvent = EventServer<ActorEvent>::Get().Subscribe( boost::bind( &CollisionSystem::OnActorEvent, this, _1 ) );
}

void CollisionSystem::OnActorEvent( ActorEvent const& Evt )
{
    if( Evt.mState == ActorEvent::Added )
    {
        Opt<ICollisionComponent> collisionC = Evt.mActor->Get<ICollisionComponent>();
        if( !collisionC.IsValid() )
        {
            return;
        }
        if ( !Evt.mActor->Get<IMoveComponent>().IsValid() )
        {
            mCollisionGrid.AddActor( Evt.mActor.Get(), 0, collisionC );
        }
    }
    else
    {
        mCollisionGrid.RemoveActor( Evt.mActor.Get() );
    }
}

namespace {
typedef std::vector<CollPair> CollVec;
CollVec collectCollisions( int id, int max, CollVec const& PossibleCollisions, CollisionStore const& mCollisionStore, double DeltaTime )
{
    CollVec rv;
    int step = PossibleCollisions.size() / max;
    int start = id * step;
    int end = ( id < max - 1 ) ? ( ( id + 1 ) * step ) : PossibleCollisions.size();
    auto i = PossibleCollisions.begin(), e = i;
    i += start;
    e += end;
    for( ; i != e; ++i )
    {
        Actor& A = *( i->A1 );
        Actor& B = *( i->A2 );
        Opt<ICollisionComponent> ACollisionC = A.Get<ICollisionComponent>();
        Opt<ICollisionComponent> BCollisionC = B.Get<ICollisionComponent>();
        BOOST_ASSERT( ACollisionC.IsValid() && BCollisionC.IsValid() ); //TODO: here this one should be true

        CollisionModel const& CollModel = mCollisionStore.GetCollisionModel( ACollisionC->GetCollisionClass(), BCollisionC->GetCollisionClass() );
        if( !CollModel.AreActorsColliding( A, B, DeltaTime ) )
        {
            continue;
        }
        rv.push_back( *i );
    }
    return rv;
}
}

void CollisionSystem::Update( double DeltaTime )
{
    mUpdateTimer.Log( "start collision" );
    mPerfTimer.Log( "pre build grid" );
    std::vector<std::pair<Opt<CollisionSubSystem>, Actor*>> collisionAndActors;
    // todo: thread pool ( clipscene ), lowpri
    for (auto actor : mScene.GetActorsFromMap( GetType_static() ))
    {
        Opt<ICollisionComponent> collisionC = actor->Get<ICollisionComponent>();
        if ( collisionC.IsValid() )
        {
            Opt<CollisionSubSystem> collisionSS = GetCollisionSubSystem( collisionC->GetId() );
            if ( collisionSS.IsValid() )
            {
                collisionAndActors.push_back( std::make_pair( collisionSS, actor ) );
                collisionSS->ClipScene( *actor );
            }
            mCollisionGrid.AddActor( actor, DeltaTime, collisionC );
        }
    }
    mPerfTimer.Log( "post build grid" );
    PossibleCollisions_t const& PossibleCollisionsSet = mCollisionGrid.GetPossibleCollisions();
    CollVec PossibleCollisions( PossibleCollisionsSet.begin(),
            PossibleCollisionsSet.end() );
    // todo: thread pool until ->Collide ( only the detection! )
    std::vector<std::future<CollVec> > ActualCollisions;

    for( int i = 0; i != 4; ++i )
    {
        ActualCollisions.push_back(
                std::async( collectCollisions, i, 4, PossibleCollisions, mCollisionStore, DeltaTime )
                );
    }
    CollVec colls;
    for( auto& f : ActualCollisions )
    {
        auto const& c = f.get();
        colls.insert( colls.end(), c.begin(), c.end() );
    }

    for( auto i = colls.begin(), e = colls.end(); i != e; ++i )
    {
        Actor& A = *( i->A1 );
        Actor& B = *( i->A2 );
        Opt<ICollisionComponent> ACollisionC = A.Get<ICollisionComponent>();
        Opt<ICollisionComponent> BCollisionC = B.Get<ICollisionComponent>();

        //TODO: needs optimization, maybe a template parameter for SubSystemHolder to subsystem would do
        Opt<CollisionSubSystem> ACollisionSS = GetCollisionSubSystem( ACollisionC->GetId() );
        if ( ACollisionSS.IsValid() )
        {
            ACollisionSS->Collide( A, B );
        }
        Opt<CollisionSubSystem> BCollisionSS = GetCollisionSubSystem( BCollisionC->GetId() );
        if ( BCollisionSS.IsValid() )
        {
            BCollisionSS->Collide( B, A );
        }
    }
    mPerfTimer.Log( "post collide" );
    // todo thread pool
    for (auto& collAndActor : collisionAndActors)
    {
        collAndActor.first->Update( *collAndActor.second, DeltaTime );
    }
    mPerfTimer.Log( "post collSS" );
    mUpdateTimer.Log( "end collision" );
}

Opt<CollisionSubSystem> CollisionSystem::GetCollisionSubSystem( int32_t id )
{
    BindIds_t& bindIds = mSubSystems.get<SubSystemHolder::AllByBindId>();
    BindIds_t::iterator subsysIt = bindIds.find( id );
    if ( subsysIt != bindIds.end() )
    {
        return Opt<CollisionSubSystem>(
                static_cast<CollisionSubSystem*>(
                    subsysIt->mSystem.Get() ) );
    }
    return nullptr;
}

} // namespace engine

