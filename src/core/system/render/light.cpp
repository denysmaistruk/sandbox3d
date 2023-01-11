#include "core/system/render/light.h"

#include "raymath.h"
#include "rlgl.h"

#define SHADER_PATH "../src/core/system/render/shaders/lights"

LightingSystem::LightingSystem(size_t) {
    m_shadowShader      = LoadShader(SHADER_PATH"/shadow.vs", SHADER_PATH"/shadow.fs");
    m_shadowMapAtlas    = LoadShadowMap(k_shadowAtlasResolution, k_shadowAtlasResolution);
    m_shadowDataBuffer  = rlLoadShaderBuffer(sizeof(Matrix) * k_shadowAtlasCellCount, nullptr, RL_DYNAMIC_DRAW);
}
