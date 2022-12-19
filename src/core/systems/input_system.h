#pragma once
#include "system_base.h"

class InputSystem : SystemBase<InputSystem>
{
    friend class SystemDebugger;
public:
    InputSystem() = default;

    void update(float dt) override;

};