#pragma once

#include <raylib.h>
#include <stdint.h>

struct ShadowMap : Texture2D {
    uint32_t    frameBufferId;
};

ShadowMap LoadShadowMap(int width, int height);

void UnloadShadowMap(ShadowMap shadowMap);
void ShadowMapBegin(ShadowMap shadowMap);
void ShadowMapEnd();

void BeginShadowCaster(Camera3D camera);
void EndShadowCaster();

Matrix CameraPerspective(Camera3D const& camera);
