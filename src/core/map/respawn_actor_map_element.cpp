#include "core/map/respawn_actor_map_element.h"
#include "spawn_actor_map_element.h"
#include "../scene.h"
#include "../i_position_component.h"
#include "../i_collision_component.h"
#include "../pickup_collision_component.h"
#include "../actor_factory.h"

namespace map {

int32_t RespawnActorMapElement::SpawnNodeId()
{
    static int32_t id = AutoId("spawn");
    return id;
}

RespawnActorMapElement::RespawnActorMapElement(int32_t Id)
    : MapElement(Id)
    , BaseInput()
    , mActorID(-1)
    , mSecsToRespawn(100)
    , mSecsToRespawnOriginal(100)
{
    AddInputNodeId(SpawnNodeId());
}

void RespawnActorMapElement::Load(Json::Value& setters)
{
    MapElement::Load(setters);

    std::string actorStr;
    if (!Json::GetStr(setters["actor"],actorStr))
    {
        return;
    }
    SetActorID(AutoId(actorStr));

    SpawnActorMapElement::LoadComponentLoaders(setters,mComponentLoaders);

    Json::GetDouble(setters["secs_to_respawn"],mSecsToRespawn);
    mSecsToRespawnOriginal=mSecsToRespawn;
}

void RespawnActorMapElement::SetActorID(int32_t actorID)
{
    mActorID=actorID;
}

int32_t RespawnActorMapElement::GetActorID()const
{
    return mActorID;
}

ActorCreator::ComponentLoaderMap_t const& RespawnActorMapElement::GetComponentLoaders()const
{
    return mComponentLoaders;
}

void RespawnActorMapElement::SetSecsToRespawn(double secsToRespawn)
{
    mSecsToRespawn=secsToRespawn;
}

double RespawnActorMapElement::GetSecsToRespawn()const
{
    return mSecsToRespawn;
}

void RespawnActorMapElement::SetSecsToRespawnOriginal(double secsToRespawnOriginal)
{
    mSecsToRespawnOriginal=secsToRespawnOriginal;
}

double RespawnActorMapElement::GetSecsToRespawnOriginal()const
{
    return mSecsToRespawnOriginal;
}

void RespawnActorMapElement::AddComponentLoader(int32_t componentId, std::auto_ptr<PropertyLoaderBase<Component> > compLoader)
{
    mComponentLoaders.insert(componentId,static_cast<ActorCreator::ComponentLoader_t *>(compLoader.release()));
}

void RespawnActorMapElement::Save(Json::Value& Element)
{
    Opt<Actor> actor(Scene::Get().GetActor(mSpawnedActorGUID));
    if (!actor.IsValid())
    {
        return;
    }
    MapElement::Save(Element);

    std::string actorName;
    if (IdStorage::Get().GetName(mActorID,actorName))
    {
        Element["actor"]=Json::Value(actorName);
    }
    Json::Value ComponentsArr(Json::arrayValue);

    Opt<IPositionComponent> positionC(actor->Get<IPositionComponent>());
    if (positionC.IsValid())
    {
        Json::Value Component(Json::objectValue);
        positionC->Save(Component);
        ComponentsArr.append(Component);
    }

    Opt<PickupCollisionComponent> pickupCollisionC(actor->Get<PickupCollisionComponent>());
    if (pickupCollisionC.IsValid())
    {
        Json::Value Component(Json::objectValue);
        pickupCollisionC->Save(Component);
        ComponentsArr.append(Component);
    }

    Element["components"]=Json::Value(ComponentsArr);
    Element["secs_to_respawn"]=mSecsToRespawnOriginal;
}

} // namespace map
