#pragma once

#include "graphics/lights.h"

struct ImGui_ImplSandbox3d_Config
{
    // Render
   /* static bool drawDepthTexture;
    static bool drawLightsDebug;
    static bool wiresMode;*/

    static bool lights[MAX_LIGHTS];

    static Vector3 shadowCasterPosition;
    static Vector3 shadowCasterTarget;
    static float shadowCasterFOV;
    static int shadowCasterCameraType;

    // Physics
    static bool pauseSimulation;
    static bool drawSceneBorders;
    static bool drawContacts;
    static int substeps;
    static float sleepEpsilon;
    static float timeScale;
};

void ImGui_ImplSandbox3d_ShowDebugWindow(bool* open);
void ImGui_ImplSandbox3d_ShowStatsWindow(bool* open);