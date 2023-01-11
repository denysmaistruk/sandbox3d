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

struct Inactive     {};
struct Position : Vector3 {};
struct LookAt   : Vector3 {};
struct DirectionalLight {};

struct ShadowCaster {
    CameraProjection projection;
};


struct PointLight {
    float lightRadius;
    float softness;
};

struct SpotLight : PointLight {
    float cutoff;
};
