#pragma once

struct VoxelObject
{
    float voxelSize;
    //Matrix transform;
    
    Mesh mesh;
    int voxelsCount;
    Material material;
    Matrix* transforms;

    /*~VoxelObject() {
        delete transforms;
    }*/
};