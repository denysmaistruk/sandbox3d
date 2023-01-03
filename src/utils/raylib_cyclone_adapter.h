#pragma once

#include "raylib.h"
#include "cyclone/core.h"

// Vector3
Vector3 toRaylib(const cyclone::Vector3& vec);
cyclone::Vector3 toCyclone(const Vector3& vec);

// Matrix
Matrix toRaylib(const cyclone::Matrix4& mat);
cyclone::Matrix4 toCyclone(const Matrix& mat);

// Quaternion
Quaternion toRaylib(const cyclone::Quaternion& q);
cyclone::Quaternion toCyclone(const Quaternion& q);