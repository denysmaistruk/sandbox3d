#pragma once
#include "system_base.h"
#include "raylib.h"

class LightningSystem : public SystemBase<LightningSystem>
{
public:
    const static int maxLights = PHYSBOX_MAX_LIGHTS;

    void update(float dt);

    void setShader(Shader& shader) { m_shader = shader; }

protected:
    LightningSystem();

private:
    Shader m_shader;
};