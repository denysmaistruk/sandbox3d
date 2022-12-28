#pragma once
#include "raylib.h"
#include "utils/graphics/lights.h"

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
    Light light;
    Camera caster;
    int lightId;
};

struct PhysComponent
{
    class CollisionBody* collBody;
};