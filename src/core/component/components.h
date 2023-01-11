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

struct LightComponent
{
    Light light;
    Camera caster;
};

struct Position {
    Vector3 value;
};

struct Inactive     {};
struct ShadowCaster {};

struct DirectionalLight {
    Vector3 target;
};

struct PointLight {
    float cutoff;
    float lightRadius;
    float softness;
};

struct SpotLight 
    : DirectionalLight
    , PointLight {};
