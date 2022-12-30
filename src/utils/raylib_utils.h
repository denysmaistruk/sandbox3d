#pragma once

struct Mesh;
struct Vector3;
struct Matrix;

Mesh genMeshPlaneTiled(float width, float length, int resX, int resZ, int tilesX, int tilesZ);
void drawGuizmo(Matrix transform);

Vector3 Vector3Translate(Vector3 v, Matrix mat);