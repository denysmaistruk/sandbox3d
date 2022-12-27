#include "render.h"

#include "imgui.h"
#include "raymath.h"

#include "core/component/components.h"
#include "core/system/lightning.h"
#include "core/camera/camera_controller.h"

#include "utils/raylib_utils.h"
#include "utils/imgui_impl_physbox.h"

extern bool ImGui_ImplRaylib_Init();
extern bool ImGui_ImplRaylib_ProcessEvent();
extern void ImGui_ImplRaylib_NewFrame();
extern void raylib_render_imgui(ImDrawData* draw_data);

RenderSystem::RenderSystem()
    : m_isWiresMode(false)
{
    m_shadowMap = LoadShadowMap(PHYSBOX_SHADOW_MAP_WIDTH, PHYSBOX_SHADOW_MAP_WIDTH);
    m_shadowShader = LoadShadowShader();
    m_geometryShader = LoadShadedGeometryShader();
    m_previewShader = LoadDepthPreviewShader();
}

void RenderSystem::update(float dt)
{
    auto entityView = getRegistry().view<TransformComponent, RenderComponent>();
    
    // Synchronize transform
    for (auto [entity, transformComponent, renderComponent] : entityView.each())
    {
        renderComponent.model.transform = transformComponent.transform;
    }
        
    // Update lightning and get shadow caster
    auto& lightSystem = LightningSystem::getSystem();
    lightSystem.update(dt);
    Camera caster = lightSystem.getShadowCaster();

    // Light matrix
    Matrix matLight = MatrixMultiply(GetCameraMatrix(caster),
        (caster.projection == CAMERA_PERSPECTIVE) ? CameraFrustum(caster) : CameraOrtho(caster));

    // Draw/update shadow map (z-buffer)
    ShadowMapBegin(m_shadowMap);
    {
        BeginShadowCaster(caster);
        {
            for (auto [entity, transformComponent, renderComponent] : entityView.each())
            {
                drawShadow(renderComponent.model);
            }
        }
        EndShadowCaster();
    }
    ShadowMapEnd();

    // Imgui new frame
    ImGuiBegin();

    // Draw scene
    BeginDrawing();
    {
        ClearBackground(RAYWHITE);
        BeginMode3D(CameraController::getCamera());
        {
            // Update values for geometry shader
            SetShaderValue(m_geometryShader, m_geometryShader.locs[SHADER_LOC_VECTOR_VIEW], (float*)&CameraController::getCamera().position, SHADER_UNIFORM_VEC3);
            SetShaderValue(m_geometryShader, m_geometryShader.locs[SHADER_LOC_AMBIENT], new float[4] { 0.4f, 0.4f, 0.4f, 1.0f }, SHADER_UNIFORM_VEC4);
            SetShaderValueMatrix(m_geometryShader, m_geometryShader.locs[SHADER_LOC_MAT_LIGHT], lightSystem.getLightMatrix());

            // Update values for preview shader
            int casterPerspective = caster.projection == CAMERA_ORTHOGRAPHIC;
            SetShaderValue(m_previewShader, m_previewShader.locs[SHADER_LOC_CASTER_PERSPECTIVE], &casterPerspective, SHADER_UNIFORM_INT);

            // Draw geometry
            for (auto [entity, transformComponent, renderComponent] : entityView.each())
            {
                drawGeometry(renderComponent.model, renderComponent.shadowFactor);
            }

            // Draw basis vector
            drawGuizmo(MatrixIdentity());

            // Draw camera target
            drawGuizmo(MatrixTranslate(CameraController::getCamera().target.x, CameraController::getCamera().target.y, CameraController::getCamera().target.z));
        }
        EndMode3D();

        BeginShaderMode(m_previewShader);
        {
            DrawTextureEx(m_shadowMap.depth, Vector2{ 0, 0 }, 0.0f, 0.125, WHITE);
        }
        EndShaderMode();

        // Imgui set up widgets and draw
        ImGuiWidgets();
        ImGuiEnd();
    }
    EndDrawing();
}

void RenderSystem::ImGuiInit()
{
    ImGui::CreateContext(nullptr);
    ImGui::StyleColorsDark(nullptr);
    ImGui_ImplRaylib_Init();

    // Build and load the texture atlas into a texture
    // (In the examples/ app this is usually done within the ImGui_ImplXXX_Init() function from one of the demo Renderer)
    struct ImGuiIO* io;
    io = &ImGui::GetIO();
    unsigned char* pixels = nullptr;

    int width = PHYSBOX_WINDOW_WIDTH;
    int height = PHYSBOX_WINDOW_HEIGHT;

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
    Texture2D texture = LoadTextureFromImage(image);
    io->Fonts->TexID = (void*)(&texture.id);
}

void RenderSystem::drawShadow(const Model& model)
{
    model.materials[0].shader = m_shadowShader;
    DrawModel(model, Vector3Zero(), 1.f, WHITE);
}

void RenderSystem::drawGeometry(const Model& model, const float shadowFactor)
{
    model.materials[0].shader = m_geometryShader;
    model.materials[0].maps[MATERIAL_MAP_SHADOW].texture = m_shadowMap.depth;
    if (m_isWiresMode)
        DrawModelWires(model, Vector3Zero(), 1.f, RED);
    else
    {
        SetShaderValue(m_geometryShader, m_geometryShader.locs[SHADER_LOC_SHADOW_FACTOR], &shadowFactor, SHADER_UNIFORM_FLOAT);
        DrawModel(model, Vector3Zero(), 1.f, WHITE);
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
    // ImGui::ShowDemoWindow(&open);  // demo example
    ImGui_ImplPhysbox_ShowDebugWindow(&open);
}