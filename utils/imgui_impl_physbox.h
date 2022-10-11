#pragma once

#include "graphics/lights.h"

struct ImGui_ImplPhysbox_Config
{
    // Render
    static bool drawDepthTexture;
    static bool drawLightsDebug;
    static bool lights[MAX_LIGHTS];

    // Physics
    static bool pauseSimulation;
    static bool drawSceneBorders;
    static bool drawContacts;
    static bool steppingMode;
    static int substeps;
    static float sleepEpsilon;
};

void ImGui_ImplPhysbox_ShowDebugWindow(bool* p_open);