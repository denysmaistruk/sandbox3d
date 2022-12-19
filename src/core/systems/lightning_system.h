#pragma once
#include "system_base.h"

class LightningSystem : public SystemBase<LightningSystem>
{
    friend class SystemDebugger;
public:
    LightningSystem() = default;

    void update(float dt) override;
};