#include "raylib.h"
#include "imgui_impl_raylib.h"

#include "raymath.h"

#include "game_scene/game_scene.h"

#include "utils/graphics/lights.h"
#include "utils/imgui_impl_physbox.h"
#include "utils/raylib_utils.h"

#define SHADOW_MAP_RESOLUTION 4096

int main(int argc, char const** argv)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1600;
    const int screenHeight = 900;

    SetConfigFlags(FLAG_MSAA_4X_HINT);  // Enable Multi Sampling Anti Aliasing 4x (if available)
    InitWindow(screenWidth, screenHeight, "physbox");

    // Define the camera to look into our 3d world
    Camera3D camera = { 0 };
    camera.position = Vector3{ 10.0f, 10.0f, 10.0f }; // Camera position
    camera.target = Vector3{ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                              // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;           // Camera mode type

    SetCameraMode(camera, CAMERA_FREE); // Set a free camera mode
    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Creating models, materials, shaders and lights
    //--------------------------------------------------------------------------------------
    GameScene::instance().init();
    GameScene::instance().update(0);
    //--------------------------------------------------------------------------------------

    // Imgui initialization
    //--------------------------------------------------------------------------------------
    ImGui::CreateContext(nullptr);
    ImGui::StyleColorsDark(nullptr);
    ImGui_ImplRaylib_Init();
    
    // Build and load the texture atlas into a texture
    // (In the examples/ app this is usually done within the ImGui_ImplXXX_Init() function from one of the demo Renderer)
    struct ImGuiIO* io;
    io = &ImGui::GetIO();
    unsigned char* pixels = nullptr;

    int width = screenWidth;
    int height = screenHeight;

    io->Fonts->TexDesiredWidth = 2048;
    io->Fonts->TexGlyphPadding = 0;
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
    //--------------------------------------------------------------------------------------

    Camera3D shadowCaster = {};
    shadowCaster.position = Vector3{ 12.5f, 100.0f, 0.0f};    // Camera position
    shadowCaster.target = Vector3{ 0.0f, 0.0f, 0.0f };         // Camera looking at point
    shadowCaster.up = Vector3{ 0.0f, 1.0f, 0.0f };             // Camera up vector (rotation towards target)
    shadowCaster.fovy = 90.0f;                                 // Camera field-of-view Y
    shadowCaster.projection = CAMERA_ORTHOGRAPHIC;             // Camera mode type

    auto matLight = MatrixMultiply(GetCameraMatrix(shadowCaster), 
        (shadowCaster.projection == CAMERA_PERSPECTIVE) ? CameraFrustum(shadowCaster) : CameraOrtho(shadowCaster));
   
    auto shadow     = LoadShadowMap(SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION);
    auto shPreview  = LoadDepthPreviewShader();
    auto shShadow   = LoadShadowShader();
    auto shGeometry = LoadShadedGeometryShader();

    // Create lights
    Light lights[MAX_LIGHTS] = { 0 };
    lights[0] = CreateLight(shGeometry, LIGHT_DIRECTIONAL, shadowCaster.position, shadowCaster.target, WHITE);
    lights[1] = CreateLight(shGeometry, LIGHT_SPOT, Vector3{ -2, 2, -2 }, Vector3Zero(), WHITE, cosf(DEG2RAD * shadowCaster.fovy * 0.46f));
    lights[2] = CreateLight(shGeometry, LIGHT_POINT, Vector3{2, 1, 2}, Vector3Zero(), YELLOW, 0.f);

    auto const drawScene = [&] (Shader const& shader) {
        
        if (!ImGui_ImplPhysbox_Config::pause) {
            GameScene::instance().update(GetFrameTime());
        }
        
        for (auto& obj : GameScene::instance().getObjects())
        {
            obj.model.materials[0].shader = shader;
            obj.model.materials[0].maps[MATERIAL_MAP_SHADOW].texture = shadow.depth;
            
            DrawModel(obj.model, Vector3Zero(), 1.f, WHITE);
        }
    };

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera);          // Update camera

        if (IsKeyDown('Z')) camera.target = Vector3{ 0.0f, 0.0f, 0.0f };
        //----------------------------------------------------------------------------------

        // Imgui start drawing
        //--------------------------------------------------------------------------------------
        ImGui_ImplRaylib_NewFrame();
        ImGui_ImplRaylib_ProcessEvent();
        ImGui::NewFrame();
        //--------------------------------------------------------------------------------------

        // Turn on/off lights by imgui input
        for (int i = 0; i < MAX_LIGHTS; ++i)
        {
            lights[i].enabled = ImGui_ImplPhysbox_Config::lights[i];
        }

        // Update light values (actually, only enable / disable them)
        for (int i = 0; i < MAX_LIGHTS; ++i) 
        {
            UpdateLightValues(shGeometry, lights[i]);
        }
       
        // Draw to shadow map (z-buffer)
        //----------------------------------------------------------------------------------
        ShadowMapBegin(shadow);
        BeginShadowCaster(shadowCaster);
            drawScene(shShadow);
        EndShadowCaster();
        ShadowMapEnd();
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);
                // Update geometry shader
                auto    const casterDir = Vector3Normalize(Vector3Subtract(shadowCaster.target, shadowCaster.position));
                SetShaderValue(shGeometry, shGeometry.locs[SHADER_LOC_VECTOR_VIEW], (float*)&camera.position, SHADER_UNIFORM_VEC3);
                SetShaderValue(shGeometry, shGeometry.locs[SHADER_LOC_AMBIENT], new float[4] { 0.4f, 0.4f, 0.4f, 1.0f }, SHADER_UNIFORM_VEC4);
                SetShaderValueMatrix(shGeometry, shGeometry.locs[SHADER_LOC_MAT_LIGHT], matLight);
                
                // Update depth preview shader
                int orthoShadowCast = shadowCaster.projection == CAMERA_ORTHOGRAPHIC;
                SetShaderValue(shPreview, shPreview.locs[SHADER_LOC_ORTHO_SHADOW_CAST], &orthoShadowCast, SHADER_UNIFORM_INT);
              
                // Draw scene geometry
                drawScene(shGeometry);
  
                // Draw debug geometry
                if (ImGui_ImplPhysbox_Config::drawDebugLights)
                {   
                    for (int i = 0; i < MAX_LIGHTS; ++i)
                    {
                        if (lights[i].enabled)
                        {
                            DrawCubeWires(lights[i].position, 0.125f, 0.125f, 0.125f, YELLOW);
                            DrawLine3D(lights[i].position, lights[i].target, LIME);
                        }
                    }
                }
                // Draw basis vector
                DrawLine3D(Vector3Zero(), Vector3{ 1.f, 0.f, 0.f }, RED);   // x
                DrawLine3D(Vector3Zero(), Vector3{ 0.f, 1.f, 0.f }, GREEN); // y
                DrawLine3D(Vector3Zero(), Vector3{ 0.f, 0.f, 1.f }, BLUE);  // z
                
                EndMode3D();

                if (ImGui_ImplPhysbox_Config::drawDebugDepth)
                {
                    BeginShaderMode(shPreview);
                        DrawTextureEx(shadow.depth, Vector2{ 0, 0 }, 0.0f, 0.125/*0.0625f*/, WHITE);
                    EndShaderMode();
                }
                
                
            // Imgui widgets
            //--------------------------------------------------------------------------------------
            bool p_open = true;
            //ImGui::ShowMetricsWindow(&p_open); // use it as an example
            ImGui_ImplPhysbox_ShowDebugWindow(&p_open);
            //--------------------------------------------------------------------------------------
          
            // Imgui end drawing
            //--------------------------------------------------------------------------------------
            ImGui::Render();
            raylib_render_imgui(ImGui::GetDrawData());
            //--------------------------------------------------------------------------------------

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    // Unload models and shaders
    for (const auto& obj : GameScene::instance().getObjects())
    {
        UnloadModel(obj.model);
    }
    UnloadShader(shPreview);
    UnloadShader(shShadow);
    UnloadShader(shGeometry);
    
    CloseWindow();             // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}