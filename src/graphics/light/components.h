#pragma once

#include "raylib.h"

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
