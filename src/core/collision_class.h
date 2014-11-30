#ifndef INCLUDED_CORE_COLLISION_CLASS_H
#define INCLUDED_CORE_COLLISION_CLASS_H
#include "platform/singleton.h"
#include <map>

class CollisionClass : public platform::Singleton<CollisionClass>
{
protected:
    friend class platform::Singleton<CollisionClass>;
    CollisionClass();
public:
    enum Type
    {
        No_Collision=0,
        Projectile,
        Creep,
        Mine,
        Player,
        Wall,
        Pickup,
        Num_Classes,
    };
    Type CollisionClass::operator()( int32_t Id ) const;
private:
    typedef std::map<int32_t,CollisionClass::Type> IdToCollClassMap_t;
    IdToCollClassMap_t mIdToCollClassMap;
};


#endif//INCLUDED_CORE_COLLISION_CLASS_H