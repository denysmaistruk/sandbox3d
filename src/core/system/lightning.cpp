#include "lightning.h"

#include "raymath.h"

#include "core/component/components.h"

LightningSystem::LightningSystem(size_t id) 
{
}

void LightningSystem::update(float dt) 
{
    auto entityView = getRegistry().view<LightComponent>();

    int lightsCount = 0;
    for (auto entity : entityView) 
    {
        ++lightsCount;
        auto& lightComponent = entityView.get<LightComponent>(entity);

        lightComponent.light.position = lightComponent.caster.position;
        lightComponent.light.target = lightComponent.caster.target;
        
        if (lightComponent.light.enabled)
        {
            m_shadowCaster = lightComponent.caster;
        }

        UpdateLightValues(m_shader, lightComponent.light);
    }

    assert(lightsCount <= maxLights);
}

void LightningSystem::setLightEnable(int lightId, bool enable)
{
    for (auto& [entity, lightComponent] : getRegistry().view<LightComponent>().each())
    {
        if (lightComponent.lightId == lightId)
        {
            lightComponent.light.enabled = enable;
        }

        lightComponent.light.enabled = true;
    }
}

Matrix LightningSystem::getLightMatrix() const
{
    return MatrixMultiply(GetCameraMatrix(m_shadowCaster),
        (m_shadowCaster.projection == CAMERA_PERSPECTIVE) ? CameraFrustum(m_shadowCaster) : CameraOrtho(m_shadowCaster));
}