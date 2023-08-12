#include "camera_controller.h"

#include "rcamera.h"
#include "sandbox3d.h"

CameraController::CameraController()
    : camera{ 0 }
{
    camera.position = Vector3{ -10.0f, 10.0f, -16.0f }; 
    camera.target = Vector3{ 5.0f, 3.0f, 8.0f };      
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };          
    camera.fovy = 45.0f;                              
    camera.projection = CAMERA_PERSPECTIVE;           
}

CameraController& CameraController::get()
{
    static CameraController controller;
    return controller;
}

Camera& CameraController::getCamera()
{
    return get().camera;
}

void CameraController::update(float dt)
{
    auto& cam = getCamera();

    // Rotation
    if (IsKeyDown(KEY_DOWN)) CameraPitch(&cam, -SANDBOX3D_CAMERA_ROTATION_SPEED * dt, false, false, false);
    if (IsKeyDown(KEY_UP)) CameraPitch(&cam, SANDBOX3D_CAMERA_ROTATION_SPEED * dt, false, false, false);
    if (IsKeyDown(KEY_RIGHT)) CameraYaw(&cam, -SANDBOX3D_CAMERA_ROTATION_SPEED * dt, false);
    if (IsKeyDown(KEY_LEFT)) CameraYaw(&cam, SANDBOX3D_CAMERA_ROTATION_SPEED * dt, false);

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    {
        CameraYaw(&cam, -GetMouseDelta().x * SANDBOX3D_CAMERA_MOUSE_MOVE_SENSITIVITY * dt, false);
        CameraPitch(&cam, -GetMouseDelta().y * SANDBOX3D_CAMERA_MOUSE_MOVE_SENSITIVITY * dt, false, false, false);
        HideCursor();
    }
    else
    {
        ShowCursor();
    }

    // Movement
    if (IsKeyDown(KEY_W)) CameraMoveForward(&cam, SANDBOX3D_CAMERA_MOVE_SPEED * dt, false);
    if (IsKeyDown(KEY_A)) CameraMoveRight(&cam, -SANDBOX3D_CAMERA_MOVE_SPEED * dt, false);
    if (IsKeyDown(KEY_S)) CameraMoveForward(&cam, -SANDBOX3D_CAMERA_MOVE_SPEED * dt, false);
    if (IsKeyDown(KEY_D)) CameraMoveRight(&cam, SANDBOX3D_CAMERA_MOVE_SPEED * dt, false);
    if (IsKeyDown(KEY_E)) CameraMoveUp(&cam, SANDBOX3D_CAMERA_MOVE_SPEED * dt);
    if (IsKeyDown(KEY_Q)) CameraMoveUp(&cam, -SANDBOX3D_CAMERA_MOVE_SPEED * dt);

    CameraMoveToTarget(&cam, -GetMouseWheelMove() * SANDBOX3D_CAMERA_MOUSE_SCROLL_SENSITIVITY * dt);
}