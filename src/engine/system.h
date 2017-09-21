#ifndef INCLUDED_ENGINE_SYSTEM_H
#define INCLUDED_ENGINE_SYSTEM_H

#include "platform/auto_id.h"
#include "platform/init.h"

#define DEFINE_SYSTEM_BASE( SystemType ) \
    static int32_t GetType_static() \
    { \
    static int32_t const typ = platform::AutoId( #SystemType ); \
    return typ; \
    } \
    virtual int32_t GetType() const \
    { \
    return SystemType::GetType_static(); \
    } \
    static bool mInitedSystem; \
    static bool initSystem() \
    { \
        mInitedSystem = true; \
        return mInitedSystem; \
    } \
 
#define INSTANTIATE_SYSTEM( SystemType ) \
namespace { \
namespace ns##SystemType { \
volatile bool inited = SystemType::initSystem(); \
} \
} \
 
#define REGISTER_SYSTEM( SystemType ) \
bool SystemType::mInitedSystem = true; \
namespace { \
struct RegisterSystem { \
void DoReg() \
{ \
    ::engine::SystemFactory::Get().RegisterSystem<SystemType>( #SystemType ); \
} \
RegisterSystem() \
{ \
    platform::Init::Get().Register( __FILE__ #SystemType, [=](){ DoReg(); } ); \
} \
} registerSystem; \
}

namespace engine {
class Engine;
class SystemEnableModifier;
class SystemFactory;
class System
{
    friend class Engine;
    friend class SystemEnableModifier;
    friend class SystemFactory;
public:
    virtual int32_t GetType() const = 0;
    virtual void Init() = 0;
    virtual void Update( double DeltaTime ) = 0;
    virtual bool IsEnabled() const;
    virtual ~System();
protected:
    virtual void SetEnabled ( bool enabled );
private:
    bool mEnabled;
};

class DefaultSystem : public System
{
public:
    DEFINE_SYSTEM_BASE( DefaultSystem )
    virtual void Init();
    virtual void Update( double DeltaTime );
};

}

#endif//INCLUDED_ENGINE_SYSTEM_H
