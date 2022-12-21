#pragma once
#include "raylib.h"

struct TransformComponent
{
    Matrix transform;
};

struct RenderComponent
{
    Model model;
    float shadowFactor;
};

struct LightComponent
{
};

namespace cyclone {
    class CollisionPrimitive;
    class CollisionPlane;
}

struct PhysComponent
{
    enum class ColliderType
    {
        Box,
        Sphere,
        Plane
    };

    ColliderType collider;
    cyclone::CollisionPrimitive* collisionBody;

    // TODO: CollisionPlane isn't in CollisionPrimitive hierarchy,
    // keep it independent for a while.
    cyclone::CollisionPlane* collisionPlane;
};