#include "platform/i_platform.h"
#include "soldier_spawn_system.h"
#include "engine.h"
#include "core/i_health_component.h"
#include "core/actor_factory.h"
#include "core/i_position_component.h"
#include "core/i_inventory_component.h"
#include "soldier_created_event.h"
#include "core/map/soldier_spawn_point_map_element_system.h"
#include "core/player_controller_component.h"

namespace engine {

SoldierSpawnSystem::SoldierSpawnSystem()
    : mScene( Scene::Get() )
    , mActorFactory( ActorFactory::Get() )
    , mPlayerAutoId(AutoId("player"))
{
}


void SoldierSpawnSystem::Init()
{
    mOnRevive=EventServer<core::ReviveEvent>::Get().Subscribe( boost::bind( &SoldierSpawnSystem::OnRevive, this, _1 ) );
}


void SoldierSpawnSystem::Update(double DeltaTime)
{
}

void SoldierSpawnSystem::OnRevive(core::ReviveEvent const& Evt)
{
    if (mEnabled)
    {
        std::auto_ptr<Actor> player(Spawn(*Evt.mClientData));       
        mScene.AddActor(player.release());
    }
}

Opt<SoldierSpawnSystem> SoldierSpawnSystem::Get()
{
    return Engine::Get().GetSystem<SoldierSpawnSystem>();
}

std::auto_ptr<Actor> SoldierSpawnSystem::Spawn(core::ClientData& clientData)
{
    map::SpawnPoints_t spawnPoints(map::SoldierSpawnPointMapElementSystem::Get()->GetActiveSpawnPoints());
    map::SpawnPoint spawnPoint(spawnPoints[rand()%spawnPoints.size()]);
    return Spawn(clientData,spawnPoint);
}

std::auto_ptr<Actor> SoldierSpawnSystem::Spawn(core::ClientData& clientData, map::SpawnPoint spawnPoint)
{
    std::auto_ptr<Actor> player(mActorFactory(mPlayerAutoId));
    return Spawn(clientData, spawnPoint, player);
}

std::auto_ptr<Actor> SoldierSpawnSystem::Spawn(core::ClientData& clientData, map::SpawnPoint spawnPoint, std::auto_ptr<Actor> player)
{
    Opt<Actor> clientActor(mScene.GetActor(clientData.mClientActorGUID));
    if(clientActor.IsValid())
    {
        Opt<IHealthComponent> healthC = clientActor->Get<IHealthComponent>();
        if (healthC.IsValid()&&healthC->IsAlive())
        {
            L1("Cannot spawn soldier for clientData (%s) current soldier still alive!\n",clientData.mClientName.c_str());
            return std::auto_ptr<Actor>();
        }
    }
    else
    {
        L2("No actor for clientData(%s). (it might be an error on revive)\n",clientData.mClientName.c_str());
    }
    Opt<IPositionComponent> positionC = player->Get<IPositionComponent>();
    positionC->SetX(spawnPoint.mX);
    positionC->SetY(spawnPoint.mY);
    //     glm::vec4 const& dimensions=mScene.GetDimensions();
    //     positionC->SetX((dimensions.x + ( rand() % ( int )( ( dimensions.z - dimensions.x ) ) )) );
    //     positionC->SetY((dimensions.y + ( rand() % ( int )( ( dimensions.w - dimensions.y ) ) )) );

    //TODO: temporary till normal inventory sync 
    Opt<IInventoryComponent> inventoryC = player->Get<IInventoryComponent>();
    if (inventoryC.IsValid())
    {
        inventoryC->SetSelectedWeapon(AutoId( "plasma_gun" ));
    }

    clientData.mClientActorGUID=player->GetGUID(); //TODO: might seek for a better place
    Opt<PlayerControllerComponent> playerControllerC(player->Get<PlayerControllerComponent>());
    if (playerControllerC.IsValid())
    {
        playerControllerC->mControllerId=clientData.mClientId;
    }
    L2("player created clientId:%d clientName:%s actorId:%d\n",clientData.mClientId,clientData.mClientName.c_str(),clientData.mClientActorGUID);
    //TODO: for debugging server:
    if (core::ProgramState::Get().mClientDatas.begin()->mClientActorGUID==player->GetGUID())
    {
        Scene::Get().SetPlayerModels(Opt<Actor>(player.get()));
    }

    EventServer<SoldierCreatedEvent>::Get().SendEvent(SoldierCreatedEvent(clientData,Opt<Actor>(player.get())));
    return player;
}


} // namespace engine
