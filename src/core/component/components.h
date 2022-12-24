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

struct PhysComponent
{
    class CollisionBody* collBody;
};