#pragma once
#include "system_base.h"
#include "raylib.h"

class LightningSystem : public SystemBase<LightningSystem>
{
    SANDBOX3D_SYSTEM_CLASS(LightningSystem);
public:
    const static int maxLights = SANDBOX3D_MAX_LIGHTS;

    void update(float dt);

    void setLightEnable(int lightId, bool enable);
    void setShader(Shader& shader) { m_shader = shader; }

    Camera getShadowCaster() const { return m_shadowCaster; }
    Matrix getLightMatrix() const;

protected:
    LightningSystem(size_t id);

private:
    Shader m_shader;
    Camera m_shadowCaster;
};