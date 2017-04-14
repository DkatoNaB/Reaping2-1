#include "platform/i_platform.h"
#include "engine/collisions/pickup_collision_sub_system.h"
#include "engine/engine.h"
#include "platform/auto_id.h"
#include "core/pickup_collision_component.h"
#include "core/i_inventory_component.h"
#include "core/i_health_component.h"
#include "core/buffs/i_buff_holder_component.h"
#include "core/buffs/buff_factory.h"
#include "core/item_type.h"
#include "../item_changed_event.h"
#include "core/i_activatable_component.h"

namespace engine {

PickupCollisionSubSystem::PickupCollisionSubSystem()
    : CollisionSubSystem()
{

}

void PickupCollisionSubSystem::Init()
{
}

void PickupCollisionSubSystem::Update( Actor& actor, double DeltaTime )
{
    auto activatableC( actor.Get<IActivatableComponent>() );
    if (!activatableC.IsValid() || !activatableC->IsActivated())
    {
        return;
    }
    auto other( mScene.GetActor( activatableC->GetActivatorGUID() ) );
    if (!other.IsValid())
    {
        return;
    }
    PickItUp( actor, *other );
}

void PickupCollisionSubSystem::Collide( Actor& actor, Actor& other )
{
    PickItUp( actor, other );
}

void PickupCollisionSubSystem::PickItUp( Actor &actor, Actor &other )
{
    Opt<PickupCollisionComponent> pickupCC = actor.Get<ICollisionComponent>();
    Opt<IInventoryComponent> inventoryC = other.Get<IInventoryComponent>();
    if (inventoryC.IsValid() && inventoryC->IsPickupItems())
    {
        int32_t prevItemId = -1;
        if (pickupCC->GetItemType() == ItemType::Weapon
            || pickupCC->GetItemType() == ItemType::Normal)
        {
            auto item( inventoryC->GetSelectedItem( pickupCC->GetItemType() ) );
            if (item.IsValid())
            {
                prevItemId = item->GetId();
            }
            inventoryC->AddItem( pickupCC->GetPickupContent() );
            inventoryC->SetSelectedItem( pickupCC->GetItemType(), pickupCC->GetPickupContent() );

        }
        else if (pickupCC->GetItemType() == ItemType::Buff)
        {
            Opt<IBuffHolderComponent> buffHolderC = other.Get<IBuffHolderComponent>();
            if (buffHolderC.IsValid())
            {
                buffHolderC->AddBuff( core::BuffFactory::Get()(pickupCC->GetPickupContent()) );
            }
        }

        EventServer<PickupEvent>::Get().SendEvent( PickupEvent( Opt<Actor>( &other ), pickupCC->GetItemType(), pickupCC->GetPickupContent() ) );
        if (prevItemId != -1)
        {
            EventServer<engine::ItemChangedEvent>::Get().SendEvent( { other.GetGUID(), pickupCC->GetItemType(),  pickupCC->GetPickupContent(), prevItemId } );
        }
        Opt<IHealthComponent> healthC = actor.Get<IHealthComponent>();
        if (healthC.IsValid())
        {
            healthC->SetHP( 0 );
        }
    }
}

} // namespace engine

