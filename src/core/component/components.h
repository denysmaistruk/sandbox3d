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
    Camera caster;
};

struct PhysComponent
{
    class CollisionBody* collBody;
};

struct DestroyTag
{
	// Each system must exclude this tag from views
};

struct ClickedEntityTag
{
    // Used for clicking and selecting entities
};