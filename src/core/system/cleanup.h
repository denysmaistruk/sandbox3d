#pragma once
#include "system_base.h"
#include "raylib.h"

class CleanupSystem : public SystemBase<CleanupSystem>
{
    SANDBOX3D_SYSTEM_CLASS(CleanupSystem);
public:
    void update(float dt);

protected:
    CleanupSystem(size_t id);
};