#include "lightning.h"
#include "core/component/components.h"

LightningSystem::LightningSystem()
    : m_shader{ 0 }
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

        if (lightComponent.light.type != LIGHT_POINT) 
        {
            lightComponent.light.position = lightComponent.caster.position;
            lightComponent.light.target = lightComponent.caster.target;
        }

        UpdateLightValues(m_shader, lightComponent.light);
    }

    assert(lightsCount <= maxLights);
}