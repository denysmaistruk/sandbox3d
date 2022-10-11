#include "raylib_cyclone_adapter.h"

// Vector3
Vector3 toRaylib(const cyclone::Vector3& vec)
{
    return { (float)vec[0], (float)vec[1], (float)vec[2] };
}

cyclone::Vector3 toCyclone(const Vector3& vec)
{
    return { vec.x, vec.y, vec.z };
}

// Matrix
Matrix toRaylib(const cyclone::Matrix4& mat)
{
    return { 
        (float)mat.data[0], (float)mat.data[1] ,(float)mat.data[2] ,(float)mat.data[3],
        (float)mat.data[4], (float)mat.data[5] ,(float)mat.data[6] ,(float)mat.data[7],
        (float)mat.data[8], (float)mat.data[9] ,(float)mat.data[10] ,(float)mat.data[11],
        0, 0, 0, 1 
    };
}

cyclone::Matrix4 toCyclone(const Matrix& mat)
{
    cyclone::Matrix4 mat4;
    mat4.data[0] = mat.m0; mat4.data[1] = mat.m4; mat4.data[2] = mat.m8; mat4.data[3] = mat.m12;
    mat4.data[4] = mat.m1; mat4.data[5] = mat.m5; mat4.data[6] = mat.m9; mat4.data[7] = mat.m13;
    mat4.data[8] = mat.m2; mat4.data[9] = mat.m6; mat4.data[10] = mat.m10; mat4.data[11] = mat.m14;
    return mat4;
}
