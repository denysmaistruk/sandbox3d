#pragma once
#include "system_base.h"

template<typename System>
struct  SystemDebugger;

class RenderSystem : SystemBase<RenderSystem>
{
    friend struct SystemDebugger<RenderSystem>;
public:
    RenderSystem() = default;

    void update(float dt);
};