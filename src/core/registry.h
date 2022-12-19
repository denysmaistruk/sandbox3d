#pragma once
#include "entt.hpp"

class EntityRegistry
{
public:
    static entt::registry& getRegistry();
};