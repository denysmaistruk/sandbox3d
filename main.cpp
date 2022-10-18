#include "raylib.h"
#include "imgui_impl_raylib.h"

#include "raymath.h"

#include "scene/scene_manager.h"
#include "scene/phys_manager.h"

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
    auto& physManager = PhysManager::instance();
    
    auto& sceneManager = SceneManager::instance();
    sceneManager.init();
    sceneManager.update(0);

    bool stepping = false;
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
    shadowCaster.position = Vector3{ 50.0f, 50.0f, 50.0f};    // Camera position
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

    // Shadow update functor
    auto const updateShadow = [&](Shader const shader) {
        for (auto& obj : sceneManager.getObjects()) {
            obj.model.materials[0].shader = shader;
            DrawModel(obj.model, Vector3Zero(), 1.f, WHITE);
        }
        // TODO: add instanced meshes to shadow map
    };

    // Main loop update functor
    auto const updateScene = [&](Shader const& shader) {
        // Simulation settings
        physManager.setSubsteps(ImGui_ImplPhysbox_Config::substeps);
        physManager.setSleepEpsilon(ImGui_ImplPhysbox_Config::sleepEpsilon);

        // Render settings
        // Shadow caster
        shadowCaster.position = ImGui_ImplPhysbox_Config::shadowCasterPosition;
        shadowCaster.target = ImGui_ImplPhysbox_Config::shadowCasterTarget;
        matLight = MatrixMultiply(GetCameraMatrix(shadowCaster),
            (shadowCaster.projection == CAMERA_PERSPECTIVE) ? CameraFrustum(shadowCaster) : CameraOrtho(shadowCaster));
        switch (ImGui_ImplPhysbox_Config::shadowCasterCameraType){
            case 0: shadowCaster.projection = CAMERA_PERSPECTIVE; break;
            case 1: shadowCaster.projection = CAMERA_ORTHOGRAPHIC; break;
        }
        shadowCaster.fovy = ImGui_ImplPhysbox_Config::shadowCasterFOV;

        // Render objects
        for (auto& obj : sceneManager.getObjects())
        {
            obj.model.materials[0].shader = shader;
            obj.model.materials[0].maps[MATERIAL_MAP_SHADOW].texture = shadow.depth;
            
            if (ImGui_ImplPhysbox_Config::wiresMode) {
                DrawModelWires(obj.model, Vector3Zero(), 1.f, WHITE);
            }
            else {
                SetShaderValue(shader, shader.locs[SHADER_LOC_SHADOW_FACTOR], &obj.shadowFactor, SHADER_UNIFORM_FLOAT);
                DrawModel(obj.model, Vector3Zero(), 1.f, WHITE);
            }
        }

#if INSTACING_ENABLED == 1
        // Render instanced objects
        BeginInstacing(shader);
        for (auto& voxelObject : sceneManager.getVoxelObjects()) {
            voxelObject.material.shader = shader;
            DrawMeshInstanced(voxelObject.mesh, voxelObject.material, voxelObject.transforms, voxelObject.instances);
        }
        EndInstancing();
#endif
        // Update positions by simulation
        if (!ImGui_ImplPhysbox_Config::pauseSimulation) {
            sceneManager.update(ImGui_ImplPhysbox_Config::timeScale * GetFrameTime());
        }
        else if (!stepping) {
            sceneManager.update(ImGui_ImplPhysbox_Config::timeScale * GetFrameTime());
            stepping = true;
        }
    };

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera);          // Update camera
        
        // Keyboard input
        //----------------------------------------------------------------------------------
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
            stepping = false;
        }
        if (IsKeyPressed(KEY_M)) {
            stepping = false;
        }
        // Pause
        if (IsKeyPressed(KEY_P)) {
            ImGui_ImplPhysbox_Config::pauseSimulation = !ImGui_ImplPhysbox_Config::pauseSimulation;
        }
        //----------------------------------------------------------------------------------

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
            if (lights[i].type == LIGHT_DIRECTIONAL) {
                lights[i].position = shadowCaster.position;
                lights[i].target = shadowCaster.target;
            }
            UpdateLightValues(shGeometry, lights[i]);
        }
       
        // Draw to shadow map (z-buffer)
        //----------------------------------------------------------------------------------
        ShadowMapBegin(shadow);
            BeginShadowCaster(shadowCaster);
                updateShadow(shShadow);
            EndShadowCaster();
        ShadowMapEnd();
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);
                // Update geometry shader
                SetShaderValue(shGeometry, shGeometry.locs[SHADER_LOC_VECTOR_VIEW], (float*)&camera.position, SHADER_UNIFORM_VEC3);
                SetShaderValue(shGeometry, shGeometry.locs[SHADER_LOC_AMBIENT], new float[4] { 0.4f, 0.4f, 0.4f, 1.0f }, SHADER_UNIFORM_VEC4);
                SetShaderValueMatrix(shGeometry, shGeometry.locs[SHADER_LOC_MAT_LIGHT], matLight);
                
                // Update depth preview shader
                int casterPerspective = shadowCaster.projection == CAMERA_ORTHOGRAPHIC;
                SetShaderValue(shPreview, shPreview.locs[SHADER_LOC_CASTER_PERSPECTIVE], &casterPerspective, SHADER_UNIFORM_INT);
              
                // Draw scene geometry
                updateScene(shGeometry);
  
                // Draw debug geometry
                if (ImGui_ImplPhysbox_Config::drawLightsDebug) {   
                    for (int i = 0; i < MAX_LIGHTS; ++i) {
                        if (lights[i].enabled) {
                            DrawCubeWires(lights[i].position, 0.125f, 0.125f, 0.125f, YELLOW);
                            DrawLine3D(lights[i].position, lights[i].target, LIME);
                        }
                    }
                }

                if (ImGui_ImplPhysbox_Config::drawSceneBorders) {
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

                if (ImGui_ImplPhysbox_Config::drawDepthTexture) {
                    BeginShaderMode(shPreview);
                        DrawTextureEx(shadow.depth, Vector2{ 0, 0 }, 0.0f, 0.125, WHITE);
                    EndShaderMode();
                }
                
                
            // Imgui widgets
            //--------------------------------------------------------------------------------------
            bool p_open = true;
            // ImGui::ShowDemoWindow(&p_open); // imgui demo example
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
    for (const auto& obj : sceneManager.getObjects()) {
        UnloadModel(obj.model);
    }
    UnloadShader(shPreview);
    UnloadShader(shShadow);
    UnloadShader(shGeometry);
    
    CloseWindow();             // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}