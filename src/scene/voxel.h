#pragma once

#include "raymath.h"

struct VoxelObject
{
    float voxel;
    Matrix transform;

    // Instancing
    Mesh mesh;
    Material material;
    int instances;
    Matrix* transforms;

    void onTransformChanged();

    VoxelObject& operator=(const VoxelObject& other)
    {
        if (this == &other) {
            return *this;
        }

        voxel = other.voxel;
        transform = other.transform;
        mesh = other.mesh;
        material = other.material;
        instances = other.instances;
        transforms = new Matrix[instances];
        for (int i = 0; i < instances; ++i) {
            transforms[i] = MatrixMultiply(other.transforms[i], MatrixInvert(other.transform));
        }
        return *this;
    }
};

inline void VoxelObject::onTransformChanged()
{
    for (int i = 0; i < instances; ++i) {
        transforms[i] = MatrixMultiply(transforms[i], transform);
    }
}
