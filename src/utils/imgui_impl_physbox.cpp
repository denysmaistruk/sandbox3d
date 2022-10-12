#include "pch.h"
#include "imgui_impl_physbox.h"

#include "imgui.h"

#include "..\src\scene\phys_manager.h"

bool ImGui_ImplPhysbox_Config::drawDepthTexture = false;

bool ImGui_ImplPhysbox_Config::drawLightsDebug = false;

bool ImGui_ImplPhysbox_Config::lights[MAX_LIGHTS] = { true, false, false, false };

bool ImGui_ImplPhysbox_Config::pauseSimulation = false;

bool ImGui_ImplPhysbox_Config::drawSceneBorders = false;

bool ImGui_ImplPhysbox_Config::drawContacts = false;

bool ImGui_ImplPhysbox_Config::steppingMode = false;

int ImGui_ImplPhysbox_Config::substeps = 1;

float ImGui_ImplPhysbox_Config::sleepEpsilon = 0.3f;

void ImGui_ImplPhysbox_ShowDebugWindow(bool* p_open)
{
    using namespace ImGui;

    ImGuiIO& io = GetIO();
    auto& physManager = PhysManager::instance();
    
    if (!Begin("Physbox/Debugger", p_open))
    {
        End();
        return;
    }

    Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    Text("%d:contacts(%d)", physManager.getContactCount(), physManager.maxContacts);
    Text("%d:rigid,%d:static(bodies)", physManager.getRigidBodiesCount(), physManager.getStaticBodiesCount());
    Text("%d:sleeping", physManager.getSleepingCount());
    
    Separator();

    // Render
    if (TreeNode("Render"))
    {
        Checkbox("DrawDepthTexture", &ImGui_ImplPhysbox_Config::drawDepthTexture);
        Checkbox("DrawLightsDebug", &ImGui_ImplPhysbox_Config::drawLightsDebug);

        // Lights
        if (TreeNode("SceneLightsSettings"))
        {
            char lightStr[32] = "Light@\0";
            for (int i = 0; i < MAX_LIGHTS; ++i)
            {
                lightStr[5] = '0' + i;
                Checkbox(lightStr, &ImGui_ImplPhysbox_Config::lights[i]);
            }

            TreePop();
        }

        TreePop();
    }
    Separator();

    // Physics
    if (TreeNode("Physics"))
    {
        Checkbox("PauseSimulation", &ImGui_ImplPhysbox_Config::pauseSimulation);
        Checkbox("DrawSceneBorders", &ImGui_ImplPhysbox_Config::drawSceneBorders);
        Checkbox("DrawContacts", &ImGui_ImplPhysbox_Config::drawContacts);
        Checkbox("SteppingMode", &ImGui_ImplPhysbox_Config::steppingMode);
        InputInt("SubstepsNum", &ImGui_ImplPhysbox_Config::substeps);
        InputFloat("SleepEpsilon", &ImGui_ImplPhysbox_Config::sleepEpsilon);

        TreePop();
    }
    Separator();

    End();
}