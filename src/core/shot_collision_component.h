#ifndef INCLUDED_CORE_SHOT_COLLISION_COMPONENT_H
#define INCLUDED_CORE_SHOT_COLLISION_COMPONENT_H

#include "core/collision_component.h"
#include "core/property_loader.h"
#include <boost/serialization/set.hpp>
#include "platform/export.h"
#include <boost/serialization/vector.hpp>

class ShotCollisionComponent : public CollisionComponent
{
public:
    virtual void SetDamage( int32_t Damage );
    virtual int32_t GetDamage() const;
    virtual void SetParentGUID( int32_t parentGUID );
    virtual int32_t GetParentGuid() const;
    virtual bool CanPassThrough( CollisionClass::Type CollType ) const;
    virtual void SetPassThrough( std::vector<CollisionClass::Type> const& CollTypes );
    virtual bool IsHitClosest();
    virtual void SetHitClosest( bool hitClosest );
    virtual bool IsDamageOnce();
    virtual void SetDamageOnce( bool damageOnce );
    virtual void SetHitCountToKill(int32_t hitCountToKill);
    virtual void SetDoDamage( bool doDamage );
    virtual bool IsDoDamage()const;
    virtual int32_t GetHitCountToKill()const;
    typedef std::vector<Opt<Actor> > ActorsCollided_t;
    virtual ActorsCollided_t& GetActorsCollided();
    typedef std::set<int32_t> Damaged_Actor_Ids_t;
    virtual void AddDamagedActorId( int32_t damagedActorId );
    virtual Damaged_Actor_Ids_t const& GetDamagedActorIds()const;
    virtual void ResetDamagedActorIds();
protected:
    ShotCollisionComponent();
    friend class ComponentFactory;
    int32_t mDamage;
    int32_t mParentGUID;
    std::vector<CollisionClass::Type> mPassThroughTypes;
    bool mHitClosest;
    Damaged_Actor_Ids_t mDamagedActorIds;
    bool mDamageOnce;
    int32_t mHitCountToKill;
    ActorsCollided_t mActorsCollided;
    bool mDoDamage;
public:
    friend class ::boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version );
};

template<class Archive>
void ShotCollisionComponent::serialize( Archive& ar, const unsigned int version )
{
    //NOTE: generated archive for this class
    ar& boost::serialization::base_object<CollisionComponent>( *this );
    ar& mDamage;
    ar& mParentGUID;
    ar& mPassThroughTypes;
    ar& mHitClosest;
    ar& mDamagedActorIds;
    ar& mDamageOnce;
}

class ShotCollisionComponentLoader: public ComponentLoader<ShotCollisionComponent>
{
public:
    DEFINE_COMPONENT_LOADER_BASE( ShotCollisionComponentLoader )
private:
    virtual void BindValues();
public:
    ShotCollisionComponentLoader();
    friend class ComponentLoaderFactory;
};


REAPING2_CLASS_EXPORT_KEY2( ShotCollisionComponent, ShotCollisionComponent, "shot_collision_component" );
#endif//INCLUDED_CORE_SHOT_COLLISION_COMPONENT_H
