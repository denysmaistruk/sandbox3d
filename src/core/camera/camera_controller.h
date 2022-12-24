#pragma once
#include "raylib.h"

class CameraController
{
public:
    static CameraController& get();
    static Camera& getCamera();

    Camera camera;

protected:
    CameraController();
};