#ifndef INCLUDED_CORE_COLLISION_COMPONENT_H
#define INCLUDED_CORE_COLLISION_COMPONENT_H
#include "core/i_collision_component.h"
#include "core/property_loader.h"

class CollisionComponent : public ICollisionComponent
{
public:
    virtual CollisionClass::Type GetCollisionClass()const;
    virtual void SetCollisionClass( CollisionClass::Type CollisionClassType );
    virtual double GetRadius()const;
    virtual void SetRadius( double Radius);

protected:
    CollisionComponent();
    friend class ComponentFactory;
    CollisionClass::Type mCollisionClassType;
    double mRadius;
};

class CollisionComponentLoader: public ComponentLoader<CollisionComponent>
{
    virtual void BindValues();
protected:
    friend class ComponentLoaderFactory;
    CollisionClass& mCollisionClass;
public:
    CollisionComponentLoader();
};

#endif//INCLUDED_CORE_COLLISION_COMPONENT_H
