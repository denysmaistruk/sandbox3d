#pragma once

#include "graphics/lights.h"

struct ImGui_ImplPhysbox_Config
{
    static bool drawDebugDepth;
    static bool drawDebugLights;

    static bool lights[MAX_LIGHTS];

    static bool pause;
};

void ImGui_ImplPhysbox_ShowDebugWindow(bool* p_open);