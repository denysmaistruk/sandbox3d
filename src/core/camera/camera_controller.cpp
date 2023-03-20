#include "camera_controller.h"

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
    UpdateCamera(&get().camera, CAMERA_FIRST_PERSON/*CAMERA_FREE*/); // TODO: free camera is broken in raylib right now
    get().camera.position = Vector3{ -10.0f, 10.0f, -16.0f };
    get().camera.target = Vector3{ 5.0f, 3.0f, 8.0f };
}