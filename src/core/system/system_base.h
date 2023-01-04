#pragma once
#include "core/registry/registry.h"

#define SANDBOX3D_DECLARE_SYSTEM(className) friend class SystemBase<className>;

struct SystemIdentifier
{
    inline static size_t identifier = 0;
    static size_t generateSystemId() { return identifier++; }
};

struct SystemTrait {};
struct NullSystem : SystemTrait {};
struct AllSystems : SystemTrait {};

template<class System>
class SystemBase
{
public:
    static System& getSystem();
    static size_t getSystemsCount();
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

template<>
inline size_t SystemBase<AllSystems>::getSystemsCount()
{
    return SystemIdentifier::identifier;
}

template<>
inline size_t SystemBase<NullSystem>::getSystemsCount()
{
    return INT_MAX;
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