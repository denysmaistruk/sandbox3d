#include "lightning.h"

#include "raymath.h"

#include "core/component/components.h"

LightningSystem::LightningSystem()
    : m_shader{ 0 }
    , m_shadowCaster{ 0 }
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

        // Turn on/off lights by imgui input
        // lightComponent.light.enabled = ImGui_ImplPhysbox_Config::lights[i];

        lightComponent.light.position = lightComponent.caster.position;
        lightComponent.light.target = lightComponent.caster.target;
        
        if (lightComponent.light.type == LIGHT_DIRECTIONAL)
        {
            m_shadowCaster = lightComponent.caster;
        }

        UpdateLightValues(m_shader, lightComponent.light);
    }

    assert(lightsCount <= maxLights);
}

Matrix LightningSystem::getLightMatrix() const
{
    return MatrixMultiply(GetCameraMatrix(m_shadowCaster),
        (m_shadowCaster.projection == CAMERA_PERSPECTIVE) ? CameraFrustum(m_shadowCaster) : CameraOrtho(m_shadowCaster));
}
