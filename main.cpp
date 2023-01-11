#include "raylib.h"

#include "imgui_impl_raylib.h"

#include "core/action.h"
#include "core/camera/camera_controller.h"
#include "core/system/cleanup.h"
#include "core/system/input/input.h"
#include "core/system/physics/physics.h"
#include "core/system/render/render.h"

int main(int argc, char const** argv)
{
    // Initialization
    SetConfigFlags(FLAG_MSAA_4X_HINT);  // enable Multi Sampling Anti Aliasing 4x (if available)
    InitWindow(SANDBOX3D_WINDOW_WIDTH, SANDBOX3D_WINDOW_HEIGHT, "sandbox3d");
    SetCameraMode(CameraController::getCamera(), CAMERA_FREE);
    SetTargetFPS(SANDBOX3D_TARGET_FPS);        
    
    // Icon
    Image icon = LoadImage(SANDBOX3D_RESOURCES_PATH"icon.png");
    SetWindowIcon(icon);

    // Create scene
    action::bindAllActions();
    action::createScene();
    
    // Imgui initialization
    ImGuiIO* io = RenderSystem::ImGuiInit();

    // Main game loop
    while (!WindowShouldClose())    // detect window close button or ESC key
    {        
        CameraController::update(GetFrameTime());

        InputSystem::getSystem().update(GetFrameTime());
        PhysSystem::getSystem().update(GetFrameTime() * 1.5f); // speed-up physics simulation
        RenderSystem::getSystem().update(GetFrameTime());
        CleanupSystem::getSystem().update(GetFrameTime());
    }

    // De-initialization
    RenderSystem::getSystem().unloadAllModels();
    RenderSystem::getSystem().unloadAllShaders();
    CloseWindow();  // close window and OpenGL context
    
    return 0;
}