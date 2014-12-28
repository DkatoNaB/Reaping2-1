#ifndef INCLUDED_CORE_PLAYER_CONTROLLER_H
#define INCLUDED_CORE_PLAYER_CONTROLLER_H

#include "input/i_input.h"
#include "core/controller_component.h"
#include "core/actor.h"
#include "core/property_loader.h"

class PlayerControllerComponent : public ControllerComponent
{
public:
    PlayerControllerComponent();
    virtual void Update( double Seconds );
};

class PlayerControllerComponentLoader: public ComponentLoader<PlayerControllerComponent>
{
    virtual void BindValues();
protected:
    PlayerControllerComponentLoader();
    friend class ComponentLoaderFactory;
};

#endif//INCLUDED_CORE_PLAYER_CONTROLLER_H
