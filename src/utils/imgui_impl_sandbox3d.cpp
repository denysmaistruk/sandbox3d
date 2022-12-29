#include "imgui_impl_sandbox3d.h"

#include "imgui.h"

#include "core/registry/registry.h"
#include "core/component/components.h"
#include "scene/phys_manager.h"

bool ImGui_ImplSandbox3d_Config::drawDepthTexture = false;

bool ImGui_ImplSandbox3d_Config::drawLightsDebug = false;

bool ImGui_ImplSandbox3d_Config::wiresMode = false;

bool ImGui_ImplSandbox3d_Config::lights[MAX_LIGHTS] = { true, false, false, false };

Vector3 ImGui_ImplSandbox3d_Config::shadowCasterPosition = Vector3{ 20.f, 70.0f, 0.0f };

Vector3 ImGui_ImplSandbox3d_Config::shadowCasterTarget = Vector3{ 0.0f, 0.0f, 0.0f };

float ImGui_ImplSandbox3d_Config::shadowCasterFOV = 90.9f;

int ImGui_ImplSandbox3d_Config::shadowCasterCameraType = 1;

bool ImGui_ImplSandbox3d_Config::pauseSimulation = false;

bool ImGui_ImplSandbox3d_Config::drawSceneBorders = false;

bool ImGui_ImplSandbox3d_Config::drawContacts = false;

int ImGui_ImplSandbox3d_Config::substeps = 1;

float ImGui_ImplSandbox3d_Config::sleepEpsilon = 0.3f;

float ImGui_ImplSandbox3d_Config::timeScale = 1.5f;

static void HelpMarker(const char* fmt, ...)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        va_list args;
        va_start(args, fmt);
        ImGui::TextV(fmt, args);
        va_end(args);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

static void fromVector3(const Vector3& vec3, float* arr3)
{
    arr3[0] = vec3.x;
    arr3[1] = vec3.y;
    arr3[2] = vec3.z;
}

static void fromArray3(const float* arr3, Vector3& vec3)
{
    vec3.x = arr3[0];
    vec3.y = arr3[1];
    vec3.z = arr3[2];
}

void ImGui_ImplSandbox3d_ShowDebugWindow(bool* open)
{
    using namespace ImGui;

    ImGuiIO& io = GetIO();
    auto& physManager = PhysManager::instance();
    
    if (!Begin("Sandbox3d/Debugger", open))
    {
        End();
        return;
    }

    //if (SmallButton("Click me!"))
    //{
    //    // do something
    //}

    Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    Text("%d:contacts(%d)", physManager.getContactCount(), physManager.maxContacts);
    Text("%d:rigid,%d:static(bodies)", physManager.getRigidBodiesCount(), physManager.getStaticBodiesCount());
    Text("%d:sleeping", physManager.getSleepingCount());
    
    Separator();

    // Render
    if (TreeNode("Render"))
    {
        Checkbox("Wires mode", &ImGui_ImplSandbox3d_Config::wiresMode);
        Checkbox("Draw depth texture", &ImGui_ImplSandbox3d_Config::drawDepthTexture);
        Checkbox("Draw lights", &ImGui_ImplSandbox3d_Config::drawLightsDebug);
        
        // Lights
        if (TreeNode("Scene lights"))
        {
            char lightStr[32] = "Light:@\0";
            for (int i = 0; i < MAX_LIGHTS; ++i)
            {
                lightStr[6] = '0' + i;
                Checkbox(lightStr, &ImGui_ImplSandbox3d_Config::lights[i]);
            }

            TreePop();
        }

        // Shadow caster 
        if (TreeNode("Shadow caster"))
        {
            float buf3[3] = { 0 };

            fromVector3(ImGui_ImplSandbox3d_Config::shadowCasterPosition, buf3);
            ImGui::InputFloat3("Position", buf3);
            fromArray3(buf3, ImGui_ImplSandbox3d_Config::shadowCasterPosition);

            fromVector3(ImGui_ImplSandbox3d_Config::shadowCasterTarget, buf3);
            ImGui::InputFloat3("Target", buf3);
            fromArray3(buf3, ImGui_ImplSandbox3d_Config::shadowCasterTarget);
            
            const char* cameraTypes[] = { "PERSPECTIVE", "ORTHOGRAPHIC" };
            ImGui::Combo("Camera type", &ImGui_ImplSandbox3d_Config::shadowCasterCameraType, cameraTypes, IM_ARRAYSIZE(cameraTypes));
            ImGui::SameLine(); HelpMarker("Affects dynamic shadowing.");

            InputFloat("FOV", &ImGui_ImplSandbox3d_Config::shadowCasterFOV);
            
            TreePop();
        }

        TreePop();
    }
    Separator();

    // Physics
    if (TreeNode("Physics"))
    {
        Checkbox("Pause simulation", &ImGui_ImplSandbox3d_Config::pauseSimulation);
        Checkbox("Draw scene borders", &ImGui_ImplSandbox3d_Config::drawSceneBorders);
        Checkbox("Draw contacts", &ImGui_ImplSandbox3d_Config::drawContacts);
        InputInt("Substeps", &ImGui_ImplSandbox3d_Config::substeps);
        InputFloat("Sleep epsilon", &ImGui_ImplSandbox3d_Config::sleepEpsilon);
        InputFloat("Time scale", &ImGui_ImplSandbox3d_Config::timeScale);
        ImGui::SameLine(); HelpMarker("Maximum dt is %.3f", physManager.getUpdateRate());

        static float friction = physManager.getCollisionFriction();
        InputFloat("Collision friction", &friction);
        if (friction != physManager.getCollisionFriction()) {
            physManager.setCollisionFriction(friction);
        }

        static float restitution = physManager.getCollisionRestitution();
        InputFloat("Collision restitution", &restitution);
        if (restitution != physManager.getCollisionRestitution()) {
            physManager.setCollisionRestitution(restitution);
        }

        static float tolerance = physManager.getCollisionTolerance();
        InputFloat("Collision tolerance", &tolerance);
        if (tolerance != physManager.getCollisionTolerance()) {
            physManager.setCollisionTolerance(tolerance);
        }

        TreePop();
    }
    Separator();

    End();
}

void ImGui_ImplSandbox3d_ShowStatsWindow(bool* open)
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize 
        | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav 
        | ImGuiWindowFlags_NoMove;
    
    const float pad = 10.0f;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 work_pos = viewport->WorkPos; // use work area to avoid menu-bar/task-bar, if any!
    ImVec2 work_size = viewport->WorkSize;
    ImVec2 window_pos, window_pos_pivot;
    window_pos.x = work_pos.x + pad;
    window_pos.y = work_pos.y + pad;
    window_pos_pivot.x = 0.0f;
    window_pos_pivot.y = 0.0f;
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);    
    ImGui::SetNextWindowBgAlpha(0.35f); // transparent background
    if (ImGui::Begin("Stats", open, window_flags))
    {
        ImGui::Text("Stats:");
        ImGui::Separator();
        ImGuiIO& io = ImGui::GetIO();
        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        auto& registry = EntityRegistry::getRegistry();
        ImGui::Text("entities - %d (lights - %d)", registry.size(), registry.view<LightComponent>().size());
    }
    ImGui::End();
}