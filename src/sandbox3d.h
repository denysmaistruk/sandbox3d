#pragma once

// Raylib settings
#define SANDBOX3D_WINDOW_WIDTH 1600
#define SANDBOX3D_WINDOW_HEIGHT 900
#define SANDBOX3D_WINDOW_ASPECT (SANDBOX3D_WINDOW_WIDTH / float(SANDBOX3D_WINDOW_HEIGHT))
#define SANDBOX3D_TARGET_FPS 60

// Lights and shadowing
#define SANDBOX3D_MAX_LIGHTS 4

#define SANDBOX3D_SHADOW_MAP_RESOLUTION 4096
#define SANDBOX3D_SHADOW_MAP_WIDTH 4096
#define SANDBOX3D_SHADOW_MAP_HEIGTH 4096

// Physics
#define SANDBOX3D_PHYSICS_SLEEP_EPSILON 0.4f

// Paths
#define SANDBOX3D_SHADER_PATH "../src/graphics/shaders/"
#define SANDBOX3D_RESOURCES_PATH "../resources/"