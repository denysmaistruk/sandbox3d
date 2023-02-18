#pragma once
#include "core/registry/registry.h"

#define SANDBOX3D_DECLARE_SYSTEM(className) friend class SystemBase<className>;
#define SYSTEMS_COUNT() (SystemID::id)

struct SystemID
{
    inline static size_t id = 0;
    static size_t generateSystemId() { return id++; }
};

template<class System>
class SystemBase
{
public:
    static System& getSystem();
    
    void update(float dt);

protected:
    entt::registry& getRegistry();
    const entt::registry& getRegistry() const;
};

template<class System>
System& SystemBase<System>::getSystem() 
{
    static size_t systemId = SystemID::generateSystemId();
    static System system(systemId);
    return system;
}

template<class System>
void SystemBase<System>::update(float dt) 
{
    static_cast<System*>(this)->update(dt);
}

template<class System>
entt::registry& SystemBase<System>::getRegistry() 
{
    return EntityRegistry::getRegistry();
}

template<class System>
const entt::registry& SystemBase<System>::getRegistry() const 
{
    return EntityRegistry::getRegistry();
}