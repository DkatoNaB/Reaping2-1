#ifndef INCLUDED_CORE_NOTIFY_PARENT_ON_DEATH_COMPONENT_H
#define INCLUDED_CORE_NOTIFY_PARENT_ON_DEATH_COMPONENT_H

#include "i_notify_parent_on_death_component.h"
#include "core/property_loader.h"

class NotifyParentOnDeathComponent : public INotifyParentOnDeathComponent
{
public:
    NotifyParentOnDeathComponent();
    virtual void SetParentGUID(int32_t parentId);
    virtual int32_t GetParentGUID()const;
    virtual void SetKillerGUID(int32_t killerId);
    virtual int32_t GetKillerGUID()const;
protected:
    friend class ComponentFactory;
    int32_t mParentGUID;
    int32_t mKillerGUID;
private:
};

class NotifyParentOnDeathComponentLoader : public ComponentLoader<NotifyParentOnDeathComponent>
{
    virtual void BindValues();
protected:
    NotifyParentOnDeathComponentLoader();
    friend class ComponentLoaderFactory;
};

#endif//INCLUDED_CORE_NOTIFY_PARENT_ON_DEATH_COMPONENT_H