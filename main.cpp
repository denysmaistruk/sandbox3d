#include "raylib.h"
#include "imgui_impl_raylib.h"

#include "raymath.h"

#include "scene/scene_manager.h"

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
    auto& sceneManager = SceneManager::instance();
    
    sceneManager.init();
    sceneManager.update(0);

    bool stepped = false;
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

    // Update method
    auto const updateScene = [&](Shader const& shader) {
        sceneManager.syncImGuiInput();

        if (!ImGui_ImplPhysbox_Config::pauseSimulation) {
            sceneManager.update(GetFrameTime());
        }
        else if (!stepped) {
            sceneManager.update(GetFrameTime());
            stepped = true;
        }
        
        for (auto& obj : sceneManager.getObjects())
        {
            obj.model.materials[0].shader = shader;
            obj.model.materials[0].maps[MATERIAL_MAP_SHADOW].texture = shadow.depth;
            
            if (!ImGui_ImplPhysbox_Config::drawInWiresMode) {
                DrawModel(obj.model, Vector3Zero(), 1.f, WHITE);
            }
            else {
                DrawModelWires(obj.model, Vector3Zero(), 1.f, WHITE);
            }
            
        }
    };

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera);          // Update camera
        
        if (IsKeyPressed(KEY_E)) {
            sceneManager.onThrowBallFromCamera(camera);
        }

        if (IsKeyPressed(KEY_R)) {
            sceneManager.onThrowBoxFromCamera(camera);
        }
        
        if (IsKeyDown(KEY_Z)) {
            camera.target = Vector3{ 0.0f, 0.0f, 0.0f };
        }

        // Camera movement
        Vector3 moveFront = Vector3Scale(Vector3Normalize(Vector3Subtract(camera.target, camera.position)), 0.5f);
        Vector3 moveSide = Vector3Scale(Vector3Normalize(Vector3CrossProduct(moveFront, camera.up)), 0.5f);
        if (IsKeyDown(KEY_W)) {
            camera.target = Vector3Add(camera.target, moveFront);
        }
        if (IsKeyDown(KEY_S)) {
            camera.target = Vector3Subtract(camera.target, moveFront);
        }
        if (IsKeyDown(KEY_D)) {
            camera.position = Vector3Add(camera.position, moveSide);
            camera.target = Vector3Add(camera.target, moveSide);
        }
        if (IsKeyDown(KEY_A)) {
            camera.position = Vector3Subtract(camera.position, moveSide);
            camera.target = Vector3Subtract(camera.target, moveSide);
        }
        // Stepping
        if (IsKeyDown(KEY_N)) {
            stepped = false;
        }
        if (IsKeyPressed(KEY_M)) {
            stepped = false;
        }
        // Pause
        if (IsKeyPressed(KEY_P)) {
            ImGui_ImplPhysbox_Config::pauseSimulation = !ImGui_ImplPhysbox_Config::pauseSimulation;
        }

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
            updateScene(shShadow);
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
                updateScene(shGeometry);
  
                // Draw debug geometry
                if (ImGui_ImplPhysbox_Config::drawLightsDebug)
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

                if (ImGui_ImplPhysbox_Config::drawSceneBorders)
                {
                    sceneManager.drawSceneBorders();
                }

                if (ImGui_ImplPhysbox_Config::drawContacts) {
                    sceneManager.drawCantacts();
                }

                // Draw basis vector
                DrawLine3D(Vector3Zero(), Vector3{ 1.f, 0.f, 0.f }, RED);   // x
                DrawLine3D(Vector3Zero(), Vector3{ 0.f, 1.f, 0.f }, GREEN); // y
                DrawLine3D(Vector3Zero(), Vector3{ 0.f, 0.f, 1.f }, BLUE);  // z

                // Draw camera target
                DrawLine3D(camera.target, Vector3Add(camera.target, Vector3{ 1.f, 0.f, 0.f }), RED);   // x
                DrawLine3D(camera.target, Vector3Add(camera.target, Vector3{ 0.f, 1.f, 0.f }), GREEN); // y
                DrawLine3D(camera.target, Vector3Add(camera.target, Vector3{ 0.f, 0.f, 1.f }), BLUE);  // z
                
                // Update camera collision with environment
                //Ray ray{camera.position, Vector3Normalize(Vector3Subtract(camera.target, camera.position))};
                //RayCollision result{ false, Vector3Length(Vector3Subtract(camera.target, camera.position)), Vector3Zero(), Vector3Zero() };
                //for (const auto& obj : sceneManager.getObjects()) {
                //    RayCollision trace = GetRayCollisionMesh(ray, obj.model.meshes[0], obj.model.transform);
                //    if (trace.hit) {
                //        result = trace;
                //    }
                //}
                //camera.target = Vector3Add(camera.position, Vector3Scale(Vector3Normalize(Vector3Subtract(ca*/mera.target, camera.position)), result.distance));


                EndMode3D();

                if (ImGui_ImplPhysbox_Config::drawDepthTexture)
                {
                    BeginShaderMode(shPreview);
                        DrawTextureEx(shadow.depth, Vector2{ 0, 0 }, 0.0f, 0.125/*0.0625f*/, WHITE);
                    EndShaderMode();
                }
                
                
            // Imgui widgets
            //--------------------------------------------------------------------------------------
            bool p_open = true;
            //ImGui::ShowDemoWindow(&p_open); // use it as an example
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
    for (const auto& obj : sceneManager.getObjects())
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