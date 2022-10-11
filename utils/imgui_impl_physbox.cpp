#include "pch.h"
#include "imgui_impl_physbox.h"

#include "imgui.h"

bool ImGui_ImplPhysbox_Config::drawDebugDepth = false;

bool ImGui_ImplPhysbox_Config::drawDebugLights = false;

bool ImGui_ImplPhysbox_Config::lights[MAX_LIGHTS] = { true, false, false, false };

bool ImGui_ImplPhysbox_Config::pause = true/*false*/;

void ImGui_ImplPhysbox_ShowDebugWindow(bool* p_open)
{
    using namespace ImGui;

    ImGuiIO& io = GetIO();
    
    if (!Begin("Physbox/Debugger", p_open))
    {
        End();
        return;
    }

    Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    
    Separator();

    Checkbox("Pause", &ImGui_ImplPhysbox_Config::pause);

    Separator();

    // Render debug
    if (TreeNode("RenderDebug"))
    {
        Checkbox("DrawDebugDepth", &ImGui_ImplPhysbox_Config::drawDebugDepth);
        Checkbox("DrawDebugLights", &ImGui_ImplPhysbox_Config::drawDebugLights);

        TreePop();
    }

    Separator();

    // Lights
    if (TreeNode("SceneLights"))
    {
        char lightStr[32] = "Light@\0";
        for (int i = 0; i < MAX_LIGHTS; ++i)
        {
            lightStr[5] = '0' + i;
            Checkbox(lightStr, &ImGui_ImplPhysbox_Config::lights[i]);
        }

        TreePop();
    }
    End();
}