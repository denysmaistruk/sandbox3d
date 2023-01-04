#include "lightning.h"

#include "raymath.h"

#include "core/component/components.h"

LightningSystem::LightningSystem(size_t id)
    : m_activeLightEntity(entt::entity(-1))
{
}

void LightningSystem::setActiveLightEntity(entt::entity lightEntity)
{
    auto const& entityView = getRegistry().view<LightComponent>(entt::exclude<DestroyTag>);
    m_activeLightEntity = entityView.contains(lightEntity) ? lightEntity : m_activeLightEntity;
}

LightComponent const& LightningSystem::getActiveLightComponent() const 
{
    auto& entityView = getRegistry().view<LightComponent>(entt::exclude<DestroyTag>);
    assert(entityView.contains(m_activeLightEntity));
    return entityView.get<LightComponent>(m_activeLightEntity);
}