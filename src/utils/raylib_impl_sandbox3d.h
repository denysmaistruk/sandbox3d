#pragma once

#include "raylib.h"
#include "raymath.h"

void DrawGuizmo(Matrix transform);

Mesh GenMeshPlaneTiled(float width, float length, int resX, int resZ, int tilesX, int tilesZ);

Vector3 Vector3Translate(Vector3 v, Matrix mat);

BoundingBox GetMeshBoundingBoxTransfomed(Mesh mesh, Matrix transform);

BoundingBox GetModelBoundingBoxTransformed(Model model);

Matrix MatrixFromFloat(float* vec);

Matrix MatrixFromFloat(float16 vec);