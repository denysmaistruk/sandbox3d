#pragma once
#include "system_base.h"
#include "raylib.h"

class LightningSystem : public SystemBase<LightningSystem>
{
    friend SystemBase<LightningSystem>;
public:
    const static int maxLights = SANDBOX3D_MAX_LIGHTS;

    void update(float dt);

    void setShader(Shader& shader) { m_shader = shader; }

    Camera getShadowCaster() const { return m_shadowCaster; }
    Matrix getLightMatrix() const;

protected:
    LightningSystem();

private:
    Shader m_shader;
    Camera m_shadowCaster;
};