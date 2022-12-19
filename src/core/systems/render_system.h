#pragma once
#include "system_base.h"

class RenderSystem : SystemBase<RenderSystem>
{
    friend class SystemDebugger;
public:
    RenderSystem() = default;

    void update(float dt) override;
};