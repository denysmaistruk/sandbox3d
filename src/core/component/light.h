#pragma once

#include "raylib.h"

struct ShadowCaster {};
struct LightSource  {};

struct DirectionalLight {};

struct PointLight {
    float lightRadius;
    float softness;
};

struct SpotLight : PointLight {
    float cutoff;
};
