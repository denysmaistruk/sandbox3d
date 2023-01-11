#pragma once

#include "raylib.h"

struct ShadowCaster {};
struct LightSource  {};

struct DirectionalLight {};

struct PointLight {
    float radius;
};

struct SpotLight : PointLight {
    float softness;
    float cutoff;
};
