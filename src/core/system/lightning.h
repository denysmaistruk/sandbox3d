#pragma once
#include "system_base.h"

#include "raylib.h"

struct LightComponent;

class LightningSystem : public SystemBase<LightningSystem>
{
    SANDBOX3D_DECLARE_SYSTEM(LightningSystem);
public:
    void setActiveLightEntity(entt::entity lightEntity);
    entt::entity getActiveLightEntity() const { return m_activeLightEntity; }
    LightComponent const& getActiveLightComponent() const;

protected:
    LightningSystem(size_t id);
    void update(float dt) {}

private:
    entt::entity m_activeLightEntity;
};
