#include "lightning.h"

#include "raymath.h"

#include "core/component/components.h"

void LightningSystem::setCurrentLightId(entt::entity lightId)
{
    auto const&     entityView = getRegistry().view<LightComponent>(entt::exclude<DestroyTag>);
    m_currentLight  = entityView.contains(lightId)
                    ? lightId : m_currentLight;
}

LightComponent const& LightningSystem::getCurrentLight() const {
    auto&   entityView = getRegistry().view<LightComponent>(entt::exclude<DestroyTag>);
    assert(entityView.contains(m_currentLight));
    return  entityView.get<LightComponent>(m_currentLight);
}
