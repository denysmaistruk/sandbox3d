#pragma once

#include <raylib.h>
#include <stdint.h>

 struct Light {
    bool enabled;
    int type;
    Vector3 position;
    Vector3 target;
    Color color;
    
    // Spot light parameters
    float cutoff;
    float lightRadius;
    float spotSoftness;

    // Shader locations
    int enabledLoc;
    int typeLoc;
    int posLoc;
    int targetLoc;
    int colorLoc;
    int cutoffLoc;
    int lightRadiusLoc;
    int spotSoftnessLoc;
};

// Light type
enum {
    LIGHT_DIRECTIONAL,
    LIGHT_POINT,
    LIGHT_SPOT
};

enum {
    MATERIAL_MAP_SHADOW = MATERIAL_MAP_BRDF + 1, // 11
};

enum {
    SHADER_LOC_MAP_SHADOW = SHADER_LOC_MAP_BRDF + 1, // 26 (max 32)
    SHADER_LOC_MAT_LIGHT,
    SHADER_LOC_AMBIENT,
    SHADER_LOC_CASTER_PERSPECTIVE, 
    SHADER_LOC_SHADOW_FACTOR,
    SHADER_LOC_INSTANCING
};

struct ShadowMap {
    uint32_t    id;
    int32_t     width;
    int32_t     height;
    Texture2D   depth;
};

Light CreateLight(Shader shader, int type, Vector3 position, Vector3 target, Color color, float cutoff = 0.7f, float lightRadius = 10.f, float spotSoftness = 0.65f);
void UpdateLightValues(Shader shader, Light light);

Shader LoadDepthPreviewShader();
Shader LoadShadowShader();
Shader LoadShadedGeometryShader();

ShadowMap LoadShadowMap(int width, int height);
void UnloadShadowMap(ShadowMap shadowMap);
void ShadowMapBegin(ShadowMap shadowMap);
void ShadowMapEnd();

void BeginShadowCaster(Camera3D camera);
void EndShadowCaster();

Matrix CameraFrustum(Camera3D const& camera);
Matrix CameraOrtho(Camera3D const& camera);

void BeginInstacing(Shader shader);
void EndInstancing();