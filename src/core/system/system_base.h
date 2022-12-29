#pragma once
#include "core/registry/registry.h"

#define SANDBOX3D_SYSTEM_CLASS(className) friend class SystemBase<className>;

struct SystemIdentifier
{
    inline static size_t identifier = 0;
    static size_t generateSystemId() { return identifier++; }
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
    static size_t systemId = SystemIdentifier::generateSystemId();
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