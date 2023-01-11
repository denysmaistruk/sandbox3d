#include "core/system/render/light.h"

#include "raymath.h"
#include "rlgl.h"
#include "external/glad.h"

#define SHADER_PATH "../src/core/system/render/shaders/lights"

LightingSystem::LightingSystem(size_t) {
    m_shadowShader      = LoadShader(SHADER_PATH"/shadow.vs", SHADER_PATH"/shadow.fs");
    m_shadowMapAtlas    = LoadShadowMap(k_shadowAtlasResolution, k_shadowAtlasResolution);
    m_lightDataBuffer   = rlLoadShaderBuffer(sizeof(m_lightDataArray), nullptr, RL_DYNAMIC_DRAW);
}

void LightingSystem::bindLightingData(Shader const& shader, Vector3 const& cameraPos)
{
    int const shadowMapAtlasId  = glGetUniformLocation(shader.id, "shadowMapAtlas");
    int const lightDataCountId  = glGetUniformLocation(shader.id, "lightDataCount");
    int const viewPositionId    = glGetUniformLocation(shader.id, "viewPos");
    int const lightDataBlockId  = glGetProgramResourceIndex(shader.id, GL_SHADER_STORAGE_BLOCK, "lightDataBlock");
    SetShaderValueTexture(shader, shadowMapAtlasId, m_shadowMapAtlas);
    SetShaderValue(shader, lightDataCountId, &m_lightDataCount, SHADER_UNIFORM_INT);
    SetShaderValue(shader, viewPositionId, (float*)&cameraPos, SHADER_UNIFORM_VEC3);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, lightDataBlockId, m_lightDataBuffer);
}
