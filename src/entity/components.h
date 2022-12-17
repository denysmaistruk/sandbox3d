#pragma once
#include "raylib.h"

struct TransformComponent
{
    Matrix transform;
};

struct ModelComponent
{
    Model model;
};

struct ShadowComponent
{
    float shadowFactor;
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

    // TODO: CollisionPlane isn't in CollisionPrimitive hierarchy
    // keep independent for a while
    cyclone::CollisionPlane* collisionPlane;
};