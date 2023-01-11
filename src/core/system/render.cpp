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

#include "graphics/text3d.h"

#include "utils/raylib_impl_sandbox3d.h"
#include "utils/imgui_impl_sandbox3d.h"
#include "utils/imguizmo_impl_sandbox3d.h"

bool ImGui_ImplRaylib_Init();
bool ImGui_ImplRaylib_ProcessEvent();
void ImGui_ImplRaylib_NewFrame();
void raylib_render_imgui(ImDrawData* draw_data);

static auto constexpr ambientColor = { 0.4f, 0.4f, 0.4f, 1.0f };

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
   /* m_shadowMapAtlas = LoadShadowMap(SANDBOX3D_SHADOW_MAP_WIDTH, SANDBOX3D_SHADOW_MAP_WIDTH);
    m_shadowMatBuffer = rlLoadShaderBuffer(sizeof(Matrix) * SANDBOX3D_SHADOW_MAP_CELL_COUNT, nullptr, RL_DYNAMIC_DRAW);
    m_lightDataBuffer = rlLoadShaderBuffer(sizeof(LightData) * SANDBOX3D_SHADOW_MAP_CELL_COUNT, nullptr, RL_DYNAMIC_DRAW);
    m_shadowShader = LoadShadowShader();
    m_geometryShader = LoadShadedGeometryShader();
    m_previewShader = LoadDepthPreviewShader();*/
    m_shadowShader = LoadShader(nullptr, nullptr);
    m_geometryShader = LoadShader(nullptr, nullptr);
    m_previewShader = LoadShader(nullptr, nullptr);
    m_text3dShader = LoadText3DShader();

    addText3dMessage("WELCOME TO SANDBOX 3D!", Vector3{ 0.f, 5.f, 0.f });
}

void RenderSystem::update(float dt)
{
    auto entityView = getRegistry().view<TransformComponent, RenderComponent>(entt::exclude<DestroyTag>);
    
    // Synchronize transform
    for (auto [entity, transformComponent, renderComponent] : entityView.each())
    {
        renderComponent.model.transform = transformComponent.transform;
    }
    
    // Update lightning and get shadow caster
    /*auto const& lightSystem = LightningSystem::getSystem();
    auto const& lightInfo = lightSystem.getActiveLightComponent();
    Camera caster = lightInfo.caster;
    Light light = lightInfo.light;
    light.position = caster.position;
    light.target = caster.target;*/

    auto const renderShadowCell = [&](Camera const& casterCamera, int const cellIndex) 
    {
        int const x = (cellIndex / SANDBOX3D_SHADOW_MAP_ROW_SIZE) * SANDBOX3D_SHADOW_MAP_CELL_SIZE;
        int const y = (cellIndex % SANDBOX3D_SHADOW_MAP_ROW_SIZE) * SANDBOX3D_SHADOW_MAP_CELL_SIZE;
        rlViewport(x, y, SANDBOX3D_SHADOW_MAP_CELL_SIZE, SANDBOX3D_SHADOW_MAP_CELL_SIZE);
        BeginShadowCaster(casterCamera);
        for (auto [entity, transformComponent, renderComponent] : entityView.each())
            drawShadow(renderComponent.model);
        EndShadowCaster();
    };

    auto const getCasterMatrix  = [](Camera const& casterCamera) {
        auto const casterWorld  = GetCameraMatrix(casterCamera);
        auto const casterProj   = CameraPerspective(casterCamera);
        return MatrixTranspose(MatrixMultiply(casterWorld, casterProj));
    };

    std::vector<Matrix> shadowCellTransform;
    std::vector<LightData> lightData;
    lightData.resize(SANDBOX3D_SHADOW_MAP_CELL_COUNT);
    shadowCellTransform.resize(SANDBOX3D_SHADOW_MAP_CELL_COUNT);

    /*{
        int shadowIndex = 0;
        Camera casterCameraProto= {};
        casterCameraProto.up    = faceUpDownVector[0];
        casterCameraProto.fovy  = 90.0f;

        auto const  monoShadowCaster = getRegistry().view<Position, LookAt, ShadowCaster>(entt::exclude<Inactive>);
        auto const  omniShadowCaster = getRegistry().view<Position, ShadowCaster>(entt::exclude<Inactive, LookAt>);

        ShadowMapBegin(m_shadowMapAtlas);
        for (auto [entity, position, lookAt, caster] : monoShadowCaster.each())
        {
            if (!(shadowIndex < SANDBOX3D_SHADOW_MAP_CELL_COUNT))
                break;
            casterCameraProto.position  = position;
            casterCameraProto.target    = lookAt;
            casterCameraProto.projection= caster.projection;
            shadowCellTransform[shadowIndex] = getCasterMatrix(casterCameraProto);
            renderShadowCell(casterCameraProto, shadowIndex++);
        }
        casterCameraProto.projection = CAMERA_PERSPECTIVE;
        for (auto [entity, position, _] : omniShadowCaster.each())
        {
            int const remains = SANDBOX3D_SHADOW_MAP_CELL_COUNT - shadowIndex;
            if ((remains / faceCount) == 0)
                break;
            casterCameraProto.position  = position;
            casterCameraProto.up = faceSideVector[0];
            for (int i = 0; i < 2; ++i) {
                casterCameraProto.target = Vector3Add(position, faceUpDownVector[i]);
                shadowCellTransform[shadowIndex] = getCasterMatrix(casterCameraProto);
                renderShadowCell(casterCameraProto, shadowIndex++);
            }
            casterCameraProto.up = faceUpDownVector[0];
            for (int i = 0; i < faceCount - 2; ++i) {
                casterCameraProto.target= Vector3Add(position, faceSideVector[i]);
                shadowCellTransform[shadowIndex] = getCasterMatrix(casterCameraProto);
                renderShadowCell(casterCameraProto, shadowIndex++);
            }
        }
        ShadowMapEnd();

        rlUpdateShaderBufferElements(m_shadowMatBuffer, shadowCellTransform.data(), sizeof(Matrix) * shadowIndex, 0);
    }

    {
        int lightIndex = 0;
        auto const directional = getRegistry().view<Position, LookAt, Color, DirectionalLight>(entt::exclude<Inactive>);
        for (auto [entity, position, lookAt, color] : directional.each())
            lightData[lightIndex++] = LightData{ LIGHT_DIRECTIONAL, position, lookAt, color };
        rlUpdateShaderBufferElements(m_lightDataBuffer, lightData.data(), sizeof(LightData) * lightIndex, 0);
    }*/

    // Imgui new frame
    ImGuiBegin();

    // Draw scene
    BeginDrawing();
    {
        ClearBackground(BLACK);
        BeginMode3D(CameraController::getCamera());
        {
            // Update values for geometry shader
            //UpdateLightValues(m_geometryShader, light);
            //SetShaderValue(m_geometryShader, m_geometryShader.locs[SHADER_LOC_VECTOR_VIEW], (float*)&CameraController::getCamera().position, SHADER_UNIFORM_VEC3);
            //SetShaderValue(m_geometryShader, m_geometryShader.locs[SHADER_LOC_AMBIENT], ambientColor.begin(), SHADER_UNIFORM_VEC4);
            //SetShaderValueTexture(m_geometryShader, m_geometryShader.locs[SHADER_LOC_MAP_SHADOW], m_shadowMapAtlas.depth);
            //
            //rlEnableShader(m_geometryShader.id);
            //// @WARNING: rlBindShaderBuffer bug
            //// slot index and buffer id order flipped
            //rlBindShaderBuffer(m_lightDataBuffer, m_geometryShader.locs[SHADER_LOC_MAT_LIGHT]);
            //rlBindShaderBuffer(m_shadowMatBuffer, m_geometryShader.locs[SHADER_LOC_MAT_SHADOW]);

            // Draw geometry
            for (auto [entity, transformComponent, renderComponent] : entityView.each())
            {
                drawGeometry(renderComponent.model, renderComponent.shadowFactor);
            }

            // Draw light sources
            /*if (m_drawLightSource)
            {
                for (auto [entity, lightComponent] : getRegistry().view<LightComponent>().each())
                {
                    if (entity == lightSystem.getActiveLightEntity())
                    {
                        drawLightSource(lightComponent.light);
                    }
                }
            }*/
            
            if (m_drawText)
            {
                BeginShaderMode(m_text3dShader);
                {  
                    drawText3D();  
                }
                EndShaderMode();
            }
            
            // All registered debug geometry
            drawDebugGeometry();

            // Highlight user selection
            //drawClicked();

            // Draw basis vector
            DrawGuizmo(MatrixIdentity());

            // Draw camera target
            DrawGuizmo(MatrixTranslate(CameraController::getCamera().target.x, CameraController::getCamera().target.y, CameraController::getCamera().target.z));
        }
        EndMode3D();
        
        if (m_drawText)
        {
            drawText();
        }

        /*if (m_drawShadowMap)
        {
            BeginShaderMode(m_previewShader);
            {
                DrawTextureEx(m_shadowMapAtlas.depth, Vector2{ 0, 0 }, 0.0f, 0.125, WHITE);
            }
            EndShaderMode();
        }*/

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
    UnloadShader(m_shadowShader);
    UnloadShader(m_geometryShader);
    UnloadShader(m_previewShader);
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

void RenderSystem::drawShadow(const Model& model)
{
    model.materials[0].shader = m_shadowShader;
    DrawModel(model, Vector3Zero(), 1.f, WHITE);
}

void RenderSystem::drawGeometry(const Model& model, const float shadowFactor)
{
    model.materials[0].shader = m_geometryShader;
    if (m_isWiresMode)
        DrawModelWires(model, Vector3Zero(), 1.f, RED);
    else
    {
        //SetShaderValue(m_geometryShader, m_geometryShader.locs[SHADER_LOC_SHADOW_FACTOR], &shadowFactor, SHADER_UNIFORM_FLOAT);
        DrawModel(model, Vector3Zero(), 1.f, WHITE);
    }
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
        Vector3 textSize = MeasureText3D(GetFontText3D(), pair.first, 10.0f, 4.0f, 0.0f);
        textSize = Vector3Multiply(textSize, Vector3{ 0.5f, 0.f, 0.5f });
        DrawText3D(GetFontText3D(), pair.first, Vector3Subtract(pair.second, textSize), 10.0f, 4.0f, 0.0f, true, WHITE, false);
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