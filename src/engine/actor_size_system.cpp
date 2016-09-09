#include "platform/i_platform.h"
#include "actor_size_system.h"
#include "core/i_collision_component.h"

namespace engine {

ActorSizeSystem::ActorSizeSystem()
    : mScene( Scene::Get() )
{
}


void ActorSizeSystem::Init()
{
    mOnActorEvent = EventServer<ActorEvent>::Get().Subscribe( boost::bind( &ActorSizeSystem::OnActorEvent, this, _1 ) );
}


void ActorSizeSystem::Update(double DeltaTime)
{
}

void ActorSizeSystem::OnActorEvent( ActorEvent const& Evt )
{
    if( Evt.mState == ActorEvent::Added )
    {
        Opt<ICollisionComponent> const collisionC = Evt.mActor->Get<ICollisionComponent>();
        mActorSize[ Evt.mActor->GetGUID() ] = ( float )( collisionC.IsValid() ? collisionC->GetRadius() : 50.0 );
    }
    else if( Evt.mState == ActorEvent::Removed )
    {
        mActorSize.erase( Evt.mActor->GetGUID() );
    }
}

float ActorSizeSystem::GetSize( uint32_t ActorGUID ) const
{
    auto it = mActorSize.find( ActorGUID );
    return it == mActorSize.end() ? 50 : it->second;
}


} // namespace engine

