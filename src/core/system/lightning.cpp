#include "lightning.h"

#include "raymath.h"

#include "core/component/components.h"

LightningSystem::LightningSystem(size_t id) 
{
}

void LightningSystem::update(float dt) 
{
    auto& entityView = getRegistry().view<LightComponent>();

    int lightsCount = 0;
    for (auto entity : entityView) 
    {
        auto& lightComponent = entityView.get<LightComponent>(entity);

        auto it = m_lightsMask.find(lightComponent.lightId);
        lightComponent.light.enabled = (it != m_lightsMask.end()) ? it->second : false;
        
        if (lightComponent.light.enabled)
        {
            m_shadowCaster = lightComponent.caster;
        }

        lightComponent.light.position = lightComponent.caster.position;
        lightComponent.light.target = lightComponent.caster.target;
                
        UpdateLightValues(m_shader, lightComponent.light);

        ++lightsCount;
    }

    assert(lightsCount <= maxLights);
}

void LightningSystem::setLightEnable(int lightId, bool enable)
{
    m_lightsMask[lightId] = enable;
}

Matrix LightningSystem::getLightMatrix() const
{
    return MatrixMultiply(GetCameraMatrix(m_shadowCaster),
        (m_shadowCaster.projection == CAMERA_PERSPECTIVE) ? CameraFrustum(m_shadowCaster) : CameraOrtho(m_shadowCaster));
}