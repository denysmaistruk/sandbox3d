#pragma once

#include "raylib.h"
#include "raymath.h"

void drawGuizmo(Matrix transform);

Mesh genMeshPlaneTiled(float width, float length, int resX, int resZ, int tilesX, int tilesZ);

Vector3 vector3Translate(Vector3 v, Matrix mat);

BoundingBox getMeshBoundingBoxTransfomed(Mesh mesh, Matrix transform);

BoundingBox getModelBoundingBoxTransformed(Model model);

Matrix matrixFromFloat(float* vec);

Matrix matrixFromFloat(float16 vec);

Model genCapsuleModel(float radius, float height, int rings, int slices);