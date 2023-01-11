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

struct PhysComponent
{
    class CollisionBody* collBody;
};

// Each system must exclude this tag from views
struct DestroyTag
{	
};

// Used for clicking and selecting entities
struct ClickedEntityTag
{ 
};

struct Inactive {};
struct Position : Vector3 {};
struct LookAt   : Vector3 {};
