#include "registry.h"

entt::registry& EntityRegistry::getRegistry()
{
    static entt::registry registry;
    return registry;
}