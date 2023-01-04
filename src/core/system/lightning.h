#pragma once
#include "system_base.h"
#include "raylib.h"

struct LightComponent;

class LightningSystem : public SystemBase<LightningSystem>
{
    SANDBOX3D_SYSTEM_CLASS(LightningSystem);
public:
    void                    setCurrentLightId   (entt::entity lightId);
    entt::entity            getCurrentLightId   () const { return m_currentLight; }
    LightComponent const&   getCurrentLight     () const;

protected:
    LightningSystem (size_t _) {}
    void update     (float dt) {}

private:
    entt::entity m_currentLight = entt::entity(-1);
};
