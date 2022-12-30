#pragma once

#include "raylib.h"

Mesh genMeshPlaneTiled(float width, float length, int resX, int resZ, int tilesX, int tilesZ);
Vector3 Vector3Translate(Vector3 v, Matrix mat);
void drawGuizmo(Matrix transform);