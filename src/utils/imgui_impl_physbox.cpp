#include "imgui_impl_physbox.h"

#include "imgui.h"

#include "..\src\scene\phys_manager.h"

bool ImGui_ImplPhysbox_Config::drawDepthTexture = false;

bool ImGui_ImplPhysbox_Config::drawLightsDebug = false;

bool ImGui_ImplPhysbox_Config::wiresMode = false;

bool ImGui_ImplPhysbox_Config::lights[MAX_LIGHTS] = { true, false, false, false };

Vector3 ImGui_ImplPhysbox_Config::shadowCasterPosition = Vector3{ 20.f, 70.0f, 0.0f };

Vector3 ImGui_ImplPhysbox_Config::shadowCasterTarget = Vector3{ 0.0f, 0.0f, 0.0f };

float ImGui_ImplPhysbox_Config::shadowCasterFOV = 90.9f;

int ImGui_ImplPhysbox_Config::shadowCasterCameraType = 1;

bool ImGui_ImplPhysbox_Config::pauseSimulation = false;

bool ImGui_ImplPhysbox_Config::drawSceneBorders = false;

bool ImGui_ImplPhysbox_Config::drawContacts = false;

int ImGui_ImplPhysbox_Config::substeps = 1;

float ImGui_ImplPhysbox_Config::sleepEpsilon = 0.3f;

float ImGui_ImplPhysbox_Config::timeScale = 1.5f;

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
        Checkbox("Wires mode", &ImGui_ImplPhysbox_Config::wiresMode);
        Checkbox("Draw depth texture", &ImGui_ImplPhysbox_Config::drawDepthTexture);
        Checkbox("Draw lights", &ImGui_ImplPhysbox_Config::drawLightsDebug);
        
        // Lights
        if (TreeNode("Scene lights"))
        {
            char lightStr[32] = "Light:@\0";
            for (int i = 0; i < MAX_LIGHTS; ++i)
            {
                lightStr[6] = '0' + i;
                Checkbox(lightStr, &ImGui_ImplPhysbox_Config::lights[i]);
            }

            TreePop();
        }

        // Shadow caster 
        if (TreeNode("Shadow caster"))
        {
            float buf3[3] = { 0 };

            fromVector3(ImGui_ImplPhysbox_Config::shadowCasterPosition, buf3);
            ImGui::InputFloat3("Position", buf3);
            fromArray3(buf3, ImGui_ImplPhysbox_Config::shadowCasterPosition);

            fromVector3(ImGui_ImplPhysbox_Config::shadowCasterTarget, buf3);
            ImGui::InputFloat3("Target", buf3);
            fromArray3(buf3, ImGui_ImplPhysbox_Config::shadowCasterTarget);
            
            const char* cameraTypes[] = { "PERSPECTIVE", "ORTHOGRAPHIC" };
            ImGui::Combo("Camera type", &ImGui_ImplPhysbox_Config::shadowCasterCameraType, cameraTypes, IM_ARRAYSIZE(cameraTypes));
            ImGui::SameLine(); HelpMarker("Affects dynamic shadowing.");

            InputFloat("FOV", &ImGui_ImplPhysbox_Config::shadowCasterFOV);
            
            TreePop();
        }

        TreePop();
    }
    Separator();

    // Physics
    if (TreeNode("Physics"))
    {
        Checkbox("Pause simulation", &ImGui_ImplPhysbox_Config::pauseSimulation);
        Checkbox("Draw scene borders", &ImGui_ImplPhysbox_Config::drawSceneBorders);
        Checkbox("Draw contacts", &ImGui_ImplPhysbox_Config::drawContacts);
        InputInt("Substeps", &ImGui_ImplPhysbox_Config::substeps);
        InputFloat("Sleep epsilon", &ImGui_ImplPhysbox_Config::sleepEpsilon);
        InputFloat("Time scale", &ImGui_ImplPhysbox_Config::timeScale);
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