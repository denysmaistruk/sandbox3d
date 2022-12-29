#include "imgui_impl_sandbox3d.h"

#include "imgui.h"

#include "core/registry/registry.h"
#include "core/component/components.h"
#include "debug/debugger_physics.h"
#include "debug/debugger_render.h"
#include "scene/phys_manager.h"

#include "core/system/lightning.h"

bool ImGui_ImplSandbox3d_Config::lights[MAX_LIGHTS] = { true, false, false, false };

struct LightsMask
{
    LightsMask()
        : lights{ false }
    {
        lights[0] = true;
    }

    void onMaskChanged()
    {
        for (int i = 0; i < LightningSystem::maxLights; ++i)
        {
            LightningSystem::getSystem().setLightEnable(i, lights[i]);
        }
    }

    bool lights[LightningSystem::maxLights];
};

static LightsMask lightsMask;

//static bool lightsMask[LightningSystem::maxLights] = { true, false, false, false };


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
    auto& physManager = PhysManager::instance();
    
    if (!ImGui::Begin("Sandbox3d", open))
    {
        ImGui::End();
        return;
    }
    
    ImGui::Text("Settings to vary sandbox behavior!");

    // Render
    if (ImGui::CollapsingHeader("Render"))
    {
        SystemDebugger<RenderSystem> renderDebugger;

        static bool wires = false;
        if (ImGui::Checkbox("Wires mode", &wires))
        {
            renderDebugger.setWiresMode(wires);
        }

        static bool shadowTexture = false;
        if (ImGui::Checkbox("Draw shadow map texture", &shadowTexture))
        {
            renderDebugger.setDrawShadowMap(shadowTexture);
        }

        static bool drawLightSource = false;
        if (ImGui::Checkbox("Draw light source", &drawLightSource))
        {
            renderDebugger.setDrawLightSource(drawLightSource);
        }
        
        // Lights
        if (ImGui::TreeNode("Scene lights"))
        {
            char lightStr[32] = "Light:@\0";
            for (int i = 0; i < LightningSystem::maxLights; ++i)
            {
                lightStr[6] = '0' + i;

                if (ImGui::Checkbox(lightStr, &lightsMask.lights[i]))
                {
                    for (int j = 0; j < LightningSystem::maxLights; ++j)
                    {
                        if (j == i)
                            continue;
                        lightsMask.lights[j] = false;
                    }
                }
            }
            lightsMask.onMaskChanged();
            ImGui::TreePop();
        }

        // Shadow caster 
        if (ImGui::TreeNode("Shadow caster"))
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

            ImGui::InputFloat("FOV", &ImGui_ImplSandbox3d_Config::shadowCasterFOV);
            
            ImGui::TreePop();
        }

        //TreePop();
    }
    //Separator();

    // Physics
    if (ImGui::CollapsingHeader("Physics"))
    {
        ImGui::Checkbox("Pause simulation", &ImGui_ImplSandbox3d_Config::pauseSimulation);
        ImGui::Checkbox("Draw scene borders", &ImGui_ImplSandbox3d_Config::drawSceneBorders);
        ImGui::Checkbox("Draw contacts", &ImGui_ImplSandbox3d_Config::drawContacts);
        ImGui::InputInt("Substeps", &ImGui_ImplSandbox3d_Config::substeps);
        ImGui::InputFloat("Sleep epsilon", &ImGui_ImplSandbox3d_Config::sleepEpsilon);
        ImGui::InputFloat("Time scale", &ImGui_ImplSandbox3d_Config::timeScale);
        ImGui::SameLine(); HelpMarker("Maximum dt is %.3f", physManager.getUpdateRate());

        static float friction = physManager.getCollisionFriction();
        ImGui::InputFloat("Collision friction", &friction);
        if (friction != physManager.getCollisionFriction()) {
            physManager.setCollisionFriction(friction);
        }

        static float restitution = physManager.getCollisionRestitution();
        ImGui::InputFloat("Collision restitution", &restitution);
        if (restitution != physManager.getCollisionRestitution()) {
            physManager.setCollisionRestitution(restitution);
        }

        static float tolerance = physManager.getCollisionTolerance();
        ImGui::InputFloat("Collision tolerance", &tolerance);
        if (tolerance != physManager.getCollisionTolerance()) {
            physManager.setCollisionTolerance(tolerance);
        }

        //TreePop();
    }
    //Separator();

    ImGui::End();
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
        ImGui::Text("fps - %.1f; %.3f ms/frame", io.Framerate, 1000.f / io.Framerate);
        auto& registry = EntityRegistry::getRegistry();
        ImGui::Text("systems - %d;", SystemIdentifier::identifier);
        ImGui::Text("entities - %d; lights - %d", registry.size(), registry.view<LightComponent>().size());

        SystemDebugger<PhysSystem> physDebugger;
        const int rigidBodies = physDebugger.getRigidBodiesCount();
        const int staticBodies = physDebugger.getStaticBodiesCount();
        const int sleepingBodies = physDebugger.getSleepingBodiesCount();
        ImGui::Text("phys bodies - %d; rigid - %d, static - %d, sleeping - %d", rigidBodies + staticBodies, rigidBodies, staticBodies, sleepingBodies);

    }
    ImGui::End();
}