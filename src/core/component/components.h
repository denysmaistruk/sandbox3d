#pragma once
#include "raylib.h"
#include "graphics/lights.h"

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
    int lightId;
    Camera caster;
};

struct PhysComponent
{
    class CollisionBody* collBody;
};