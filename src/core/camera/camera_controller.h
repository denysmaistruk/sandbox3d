#pragma once
#include "raylib.h"

class CameraController
{
public:
    static CameraController& get();
    static Camera& getCamera();
    static void update(float dt);

    Camera camera;

protected:
    CameraController();
};