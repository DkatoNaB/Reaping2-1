#include "i_core.h"
#include "core/i_position_component.h"
#include "core/i_move_component.h"

Weapon::Weapon( int32_t Id )
    : Item( Id )
    , mCooldown( 0.0 )
    , mShootCooldown( 1.0 )
    , mShootAltCooldown( 1.0 )
    , mScatter( 0 )
    , mAltScatter( 0 )
{
    mType = Item::Weapon;
}
void Weapon::Update( double Seconds )
{
    Item::Update( Seconds );
    double cd = mCooldown;
    cd -= Seconds;
    if( cd < 0 )
    {
        cd = 0;
    }
    mCooldown = cd;
}

void Weapon::Shoot()
{
    if( !mActor )
    {
        return;
    }
    if( mCooldown != 0.0 )
    {
        return;
    }

    Projectiles_t Projectiles;
    ShootImpl( Projectiles );
    Scene& Scen( Scene::Get() );
    double actorOrientation = mActor->Get<IPositionComponent>()->GetOrientation();
    if( mScatter )
    {
        actorOrientation += ( rand() % mScatter - mScatter / 2. ) * 0.01 * boost::math::double_constants::pi;
    }
    for( Projectiles_t::iterator i = Projectiles.begin(), e = Projectiles.end(); i != e; ++i )
    {
        Shot& Proj = *i;
        Opt<IPositionComponent> projPositionC = Proj.Get<IPositionComponent>();
        Opt<IPositionComponent> actorPositionC = mActor->Get<IPositionComponent>();
        projPositionC->SetX( actorPositionC->GetX() );
        projPositionC->SetY( actorPositionC->GetY() );
        Proj.SetParent( *mActor );
        projPositionC->SetOrientation( projPositionC->GetOrientation() + actorOrientation );
        Proj.Get<IMoveComponent>()->SetHeading( projPositionC->GetOrientation() );
        Scen.AddActor( &Proj );
    }
    Projectiles.release().release();

    mCooldown = mShootCooldown;
}

void Weapon::ShootAlt()
{
    if( !mActor )
    {
        return;
    }
    if( mCooldown != 0.0 )
    {
        return;
    }

    Projectiles_t Projectiles;
    ShootAltImpl( Projectiles );
    Scene& Scen( Scene::Get() );
    Opt<IPositionComponent> actorPositionC = mActor->Get<IPositionComponent>();
    double actorOrientation = actorPositionC->GetOrientation();
    if( mAltScatter )
    {
        actorOrientation += ( rand() % mAltScatter - mAltScatter / 2. ) * 0.01 * boost::math::double_constants::pi;
    }
    for( Projectiles_t::iterator i = Projectiles.begin(), e = Projectiles.end(); i != e; ++i )
    {
        Shot& Proj = *i;
        Opt<IPositionComponent> projPositionC = Proj.Get<IPositionComponent>();
        projPositionC->SetX( actorPositionC->GetX() );
        projPositionC->SetY( actorPositionC->GetY() );
        Proj.SetParent( *mActor );
        projPositionC->SetOrientation( projPositionC->GetOrientation() + actorOrientation );
        Proj.Get<IMoveComponent>()->SetHeading( projPositionC->GetOrientation() );
        Scen.AddActor( &Proj );
    }
    Projectiles.release().release();

    mCooldown = mShootAltCooldown;
}
