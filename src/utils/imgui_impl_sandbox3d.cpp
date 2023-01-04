#include "imgui_impl_sandbox3d.h"

#include "imgui.h"

#include "core/component/components.h"
#include "core/registry/registry.h"
#include "core/system/lightning.h"
#include "core/system/physics/collision.h"

#include "debug/debugger_physics.h"
#include "debug/debugger_render.h"

#include "utils/raylib_impl_sandbox3d.h"

// TODO: simplify this function
static void fetchSleepingEntities(bool enable)
{
    static std::map<entt::entity, int> entMsgKeyMap;
    
    auto view = EntityRegistry::getRegistry().view<TransformComponent, PhysComponent>(entt::exclude<DestroyTag>);

    if (!enable)
    {
        if (!entMsgKeyMap.empty())
        {
            for (auto [entity, index] : entMsgKeyMap)
            {
                RenderSystem::getSystem().removeTextMessage(index);
            }
            entMsgKeyMap.clear();
        }

        return;
    }

    for (auto& [entity, transformComponent, physComponent] : view.each())
    {
        const char* msg = "Zzz!";
        const Vector3 msgPos = Vector3Add(Vector3Translate(Vector3Zero(), transformComponent.transform), Vector3{ 0.f, 1.f, 0.f });

        if (physComponent.collBody->isSleeping())
        {
            if (entMsgKeyMap.find(entity) == entMsgKeyMap.end())
            {
                // Add new
                entMsgKeyMap[entity] = RenderSystem::getSystem().addTextMessage(msg, msgPos);
            }
            else
            {
                // Update existing
                RenderSystem::getSystem().updateTextMessage(entMsgKeyMap[entity], msg, msgPos);
            }
        }
        else // not sleeping
        {
            if (entMsgKeyMap.find(entity) != entMsgKeyMap.end())
            {
                RenderSystem::getSystem().removeTextMessage(entMsgKeyMap[entity]);
                entMsgKeyMap.erase(entMsgKeyMap.find(entity));
            }
        }
    }
}

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

void ImGui_ImplSandbox3d_ShowDebugWindow(bool* open)
{
    if (!ImGui::Begin("Sandbox3d", open))
    {
        ImGui::End();
        return;
    }

    ImGui::Text("Settings to vary sandbox behavior!");

    // Render
    if (ImGui::CollapsingHeader("Render System"))
    {
        SystemDebugger<RenderSystem> renderDebugger;

        static bool drawText = true;
        if (ImGui::Checkbox("Draw Text", &drawText))
        {
            renderDebugger.setDrawText(drawText);
        }

        static bool wires = false;
        if (ImGui::Checkbox("Wires mode", &wires))
        {
            renderDebugger.setWiresMode(wires);
        }

        static bool shadowTexture = false;
        if (ImGui::Checkbox("Shadow map texture", &shadowTexture))
        {
            renderDebugger.setDrawShadowMap(shadowTexture);
        }

        static bool drawLightSource = false;
        if (ImGui::Checkbox("Light source", &drawLightSource))
        {
            renderDebugger.setDrawLightSource(drawLightSource);
        }
        
        // Lights
        if (ImGui::TreeNode("Lights"))
        {
            auto const& lights = EntityRegistry::getRegistry().view<LightComponent>();
            static int current = lights.find(LightningSystem::getSystem().getActiveLightEntity()) - lights.begin();
            char lightStr[32] = "Light:@\0";
            int i = 0;
            for (auto const& entity : lights)
            {
                lightStr[6] = '0' + i;
                if (ImGui::RadioButton(lightStr, &current, i))
                {
                    LightningSystem::getSystem().setActiveLightEntity(entity);
                }
                ++i;
            }
            ImGui::TreePop();
        }
    }
    
    // Physics
    if (ImGui::CollapsingHeader("Physics System"))
    {
        static bool runPhysics = true;
        ImGui::PushStyleColor(ImGuiCol_Button, runPhysics ? (ImVec4)ImColor(0.75f, 0.25f, 0.25f) : (ImVec4)ImColor(0.1f, 0.45f, 0.1f));
        if (ImGui::Button(runPhysics ? "Stop Physics" : "Run Physics", { 100, 20 }))
        {
            runPhysics = !runPhysics;
            SystemDebugger<PhysSystem>().setUpdatePaused(!runPhysics);
        }
        ImGui::PopStyleColor();

        static bool drawSleepingMarkers = false;
        if (ImGui::Checkbox("Draw sleeping markers", &drawSleepingMarkers))
        {
            // value was changed
        }
        fetchSleepingEntities(drawSleepingMarkers);

        static bool drawContacts = false;
        if (ImGui::Checkbox("Draw contacts", &drawContacts))
        {
            static int callbackIndex = 0;
            if (drawContacts)
            {
                callbackIndex = RenderSystem::getSystem().addDebugDrawCallback([]() 
                    { 
                        SystemDebugger<PhysSystem>().drawContacts(); 
                    });
            }
            else
            {
                RenderSystem::getSystem().removeDebugDrawCallback(callbackIndex);
            }
        }
        auto params = PhysSystem::getSystem().getParams();
        bool paramsChanged = false;

        if (ImGui::InputInt("Substeps", &params.substeps))
        {
            paramsChanged = true;
        }
        
        if (ImGui::InputFloat("Update rate", &params.updateRate))
        {
            paramsChanged = true;
        }
        
        if (ImGui::InputFloat("Friction", &params.friction))
        {
            paramsChanged = true;
        }
        
        if (ImGui::InputFloat("Restitution", &params.restitution))
        {
            paramsChanged = true;
        }
      
        if (ImGui::InputFloat("Tolerance", &params.tolerance))
        {
            paramsChanged = true;
        }
        
        if (paramsChanged)
        {
            PhysSystem::getSystem().setParams(params);
        }
    }
    
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
        ImGui::Text("systems - %d;", SystemBase<AllSystems>::getSystemsCount());
        ImGui::Text("entities - %d; [lights - %d]", registry.alive(), registry.view<LightComponent>().size());

        SystemDebugger<PhysSystem> physDebugger;
        const int rigidBodies = physDebugger.getRigidBodiesCount();
        const int staticBodies = physDebugger.getStaticBodiesCount();
        const int sleepingBodies = physDebugger.getSleepingBodiesCount();
        ImGui::Text("phys bodies - %d; [rigid - %d, static - %d, sleeping - %d]", rigidBodies + staticBodies, rigidBodies, staticBodies, sleepingBodies);
        ImGui::Text("contacts - %d;", physDebugger.getContactsCount());
    }
    ImGui::End();
}