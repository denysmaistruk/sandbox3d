#include "render.h"

#include "imgui.h"
#include "imguizmo.h"
#include "raymath.h"
#include "rlgl.h"

#include "debug/debugger_physics.h"

#include "core/component/components.h"
#include "core/system/physics/collision.h"
#include "core/system/physics/physics.h"
#include "core/camera/camera_controller.h"
#include "core/system/render/light.h"

#include "text3d.h"

#include "utils/raylib_impl_sandbox3d.h"
#include "utils/imgui_impl_sandbox3d.h"
#include "utils/imguizmo_impl_sandbox3d.h"

bool ImGui_ImplRaylib_Init();
bool ImGui_ImplRaylib_ProcessEvent();
void ImGui_ImplRaylib_NewFrame();
void raylib_render_imgui(ImDrawData* draw_data);

#define SHADER_LIGHTS_PATH "../src/core/system/render/shaders/lights"

static constexpr int faceCount = 6;
static constexpr Vector3 faceSideVector[] = {
    Vector3{ 1.0f, 0.0f, 0.0f}, // right
    Vector3{-1.0f, 0.0f, 0.0f}, // left
    Vector3{0.0f, 0.0f,  1.0f}, // front
    Vector3{0.0f, 0.0f, -1.0f}, // back
};

static constexpr Vector3 faceUpDownVector[] = {
    Vector3{ 0.0f,  1.0f, 0.0f },
    Vector3{ 0.0f, -1.0f, 0.0f },
};

struct LightData
{
    int type;
    Vector3 position;
    Vector3 target;
    Color color;
    float cutoff;
    float lightRadius;
    float spotSoftness;
};

RenderSystem::RenderSystem(size_t id)
    : m_isWiresMode(false)
    , m_drawShadowMap(false)
    , m_drawLightSource(false)
    , m_drawText(true)
{
    m_geometryShader    = LoadShader(SHADER_LIGHTS_PATH"/geom.vs", SHADER_LIGHTS_PATH"/geom.fs");
    m_text3dShader      = loadText3DShader();
    addText3dMessage("WELCOME TO SANDBOX 3D!", Vector3{ 0.f, 5.f, 0.f });
}

void RenderSystem::update(float dt)
{
    auto entityView = getRegistry().view
        < TransformComponent
        , RenderComponent >
        ( entt::exclude<DestroyTag> );
    
    // Synchronize transform
    for (auto [entity, transformComponent, renderComponent] : entityView.each())
        renderComponent.model.transform = transformComponent.transform;

    // Imgui new frame
    ImGuiBegin();

    auto const drawFunction = m_isWiresMode ? DrawModelWires : DrawModel;
    auto const drawScene    = [&](auto draw) {
        for (auto [entity, transformComponent, renderComponent] : entityView.each())
            draw(renderComponent.model);
    };

    auto const  &mainCamera     = CameraController  ::getCamera();
    auto        &lightingSystem = LightingSystem    ::getSystem();
    
    lightingSystem.prepareLightingData(drawScene);

    // Draw scene
    BeginDrawing();
    {
        ClearBackground(BLACK);
        
        BeginMode3D(mainCamera);
        {
            // Update values for geometry shader
            lightingSystem.bindLightingData(m_geometryShader, mainCamera.position);

            drawScene([&](Model const& model) {
                model.materials[0].shader = m_geometryShader;
                drawFunction(model, Vector3Zero(), 1.f, WHITE);
            });

            // Draw light sources
            if (m_drawLightSource)
                for (auto [entity, position, color] : getRegistry().view<Position, Color, LightSource>().each())
                    DrawCubeWires(position, 0.125f, 0.125f, 0.125f, color);

            if (m_drawText)
            {
                BeginShaderMode(m_text3dShader);
                drawText3D();
                EndShaderMode();
            }
            
            // All registered debug geometry
            drawDebugGeometry();

            // Highlight user selection
            //drawClicked();

            // Draw basis vector
            DrawGuizmo(MatrixIdentity());

            // Draw camera target
            DrawGuizmo(MatrixTranslate(mainCamera.target.x, mainCamera.target.y, mainCamera.target.z));
        }
        EndMode3D();
        
        if (m_drawText)
        {
            drawText();
        }

        if (m_drawShadowMap)
        {
            DrawTextureEx(lightingSystem.getShadowMapAtlasTexture(), Vector2{0, 0}, 0.0f, 0.125, WHITE);
        }

        // Imgui set up widgets and draw
        ImGuiWidgets();
        ImGuizmoWidgets();
        ImGuiEnd();
    }
    EndDrawing();
}

void RenderSystem::unloadAllModels()
{
    auto entityView = getRegistry().view<RenderComponent>();

    for (auto [entity,  renderComponent] : entityView.each())
    {
        UnloadModel(renderComponent.model);
    }
}

void RenderSystem::unloadAllShaders()
{
    UnloadShader(m_geometryShader);
}

int RenderSystem::addDebugDrawCallback(const std::function<void()>& callBack)
{
    static int key = 0;
    m_debugDrawCallbacks[key] = callBack;
    return key++;
}

void RenderSystem::removeDebugDrawCallback(int key)
{
    m_debugDrawCallbacks.erase(m_debugDrawCallbacks.find(key));
}

int RenderSystem::addTextMessage(const char* message, const Vector3& pos)
{
    static int key = 0;
    m_textMessages[key] = std::make_pair(message, pos);
    return key++;
}

void RenderSystem::updateTextMessage(int key, const char* newMessage, const Vector3& newPos)
{
    assert(m_textMessages.find(key) != m_textMessages.end());
    m_textMessages[key] = std::make_pair(newMessage, newPos);
}

void RenderSystem::removeTextMessage(int key)
{
    assert(m_textMessages.find(key) != m_textMessages.end());
    m_textMessages.erase(m_textMessages.find(key));
}

int RenderSystem::addText3dMessage(const char* message, const Vector3& pos)
{
    static int key = 0;
    m_text3dMessages[key] = std::make_pair(message, pos);
    return key++;
}

void RenderSystem::updateText3dMessage(int key, const char* newMessage, const Vector3& newPos)
{
    assert(m_text3dMessages.find(key) != m_text3dMessages.end());
    m_text3dMessages[key] = std::make_pair(newMessage, newPos);
}

void RenderSystem::removeText3dMessage(int key)
{
    assert(m_text3dMessages.find(key) != m_text3dMessages.end());
    m_text3dMessages.erase(m_text3dMessages.find(key));
}

ImGuiIO* RenderSystem::ImGuiInit()
{
    ImGui::CreateContext(nullptr);
    ImGui::StyleColorsDark(nullptr);
    ImGui_ImplRaylib_Init();

    // Build and load the texture atlas into a texture
    // (In the examples/ app this is usually done within the ImGui_ImplXXX_Init() function from one of the demo Renderer)
    ImGuiIO* io;
    io = &ImGui::GetIO();
    unsigned char* pixels = nullptr;

    int width = SANDBOX3D_WINDOW_WIDTH;
    int height = SANDBOX3D_WINDOW_HEIGHT;

    io->Fonts->TexDesiredWidth = 2048;
    io->Fonts->TexGlyphPadding = 1;
    io->Fonts->Flags |= ImFontAtlasFlags_NoPowerOfTwoHeight;

    io->Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, nullptr);

    // At this point you've got the texture data and you need to upload that your your graphic system:
    // After we have created the texture, store its pointer/identifier (_in whichever format your engine uses_) in 'io.Fonts->TexID'.
    // This will be passed back to your via the renderer. Basically ImTextureID == void*. Read FAQ for details about ImTextureID.
    Image image = {};
    image.data = pixels;
    image.width = width;
    image.height = height;
    image.mipmaps = 1;
    image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    static Texture2D texture = LoadTextureFromImage(image);
    io->Fonts->TexID = (void*)(&texture.id);
    return io;
}

void RenderSystem::drawText()
{
    for (const auto& [key, pair] : m_textMessages)
    {
        const char* text = pair.first;
        Vector2 pos = GetWorldToScreen(pair.second, CameraController::getCamera());
        DrawText(text, pos.x, pos.y, 20.f, YELLOW);
    }
}

void RenderSystem::drawText3D()
{
    for (const auto& [key, pair] : m_text3dMessages)
    {
        Vector3 textSize = measureText3D(getFontText3D(), pair.first, 10.0f, 4.0f, 0.0f);
        textSize = Vector3Multiply(textSize, Vector3{ 0.5f, 0.f, 0.5f });
        drawText3DImpl(getFontText3D(), pair.first, Vector3Subtract(pair.second, textSize), 10.0f, 4.0f, 0.0f, true, WHITE, false);
    }
}

void RenderSystem::drawDebugGeometry()
{
    for (const auto& [key, callback] : m_debugDrawCallbacks)
    {
        callback();
    }
}

void RenderSystem::drawClicked()
{
    auto& entityView = EntityRegistry::getRegistry().view<RenderComponent, ClickedEntityTag>(entt::exclude<DestroyTag>);
    for (auto [entity, renderComponent] : entityView.each())
    {
        DrawBoundingBox(GetModelBoundingBoxTransformed(renderComponent.model), WHITE);
    }
}

void RenderSystem::ImGuiBegin()
{
    ImGui_ImplRaylib_NewFrame();
    ImGui_ImplRaylib_ProcessEvent();
    ImGui::NewFrame();
}

void RenderSystem::ImGuiEnd()
{
    ImGui::Render();
    raylib_render_imgui(ImGui::GetDrawData());
}

void RenderSystem::ImGuiWidgets()
{
    bool open = true;
    //ImGui::ShowDemoWindow(&open);  // demo example
    ImGui_ImplSandbox3d_ShowDebugWindow(&open);
    ImGui_ImplSandbox3d_ShowStatsWindow(&open);
}

void RenderSystem::ImGuizmoBegin()
{
    ImGuizmo::BeginFrame();
}

void RenderSystem::ImGuizmoWidgets()
{
    ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

    bool open = true;
    ImGui::Begin("Manipulate", &open);
    
    bool clicked = false;
    for (auto& [entity, transformComponent] : EntityRegistry::getRegistry().view<TransformComponent, ClickedEntityTag>().each())
    {
        clicked = true;
        ImGuizmo_ImplSandbox3d_EditTransform(CameraController::getCamera(), transformComponent.transform);
    }

    if (!clicked)
    {
        ImGui::Text("Click the object to manipulate [Ctrl+MouseL]");
    }

    ImGui::End();
}