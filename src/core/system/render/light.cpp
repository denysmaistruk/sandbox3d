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
    int const lightInfoId       = glGetUniformLocation(shader.id, "lightingInfo");
    int const viewPositionId    = glGetUniformLocation(shader.id, "viewPos");
    int const lightDataBlockId  = glGetProgramResourceIndex(shader.id, GL_SHADER_STORAGE_BLOCK, "lightDataBlock");
    int const lightInfo[]       = {m_lightDataCount, k_shadowAtlasRowSize, k_shadowAtlasCellResolution};
    SetShaderValueTexture(shader, shadowMapAtlasId, m_shadowMapAtlas);
    SetShaderValue(shader, lightInfoId      , &lightInfo, SHADER_UNIFORM_IVEC3);
    SetShaderValue(shader, viewPositionId   , &cameraPos, SHADER_UNIFORM_VEC3);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, lightDataBlockId, m_lightDataBuffer);
}
