#pragma once
#include "core/registry.h"

template<class System>
class SystemBase
{
public:
    static System& get();
    virtual void update(float dt);

protected:
    entt::registry& getRegistry();
    const entt::registry& getRegistry() const;
};

template<class System>
System& SystemBase<System>::get()
{
    static System system;
    return system;
}
template<class System>
void SystemBase<System>::update(float dt)
{
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