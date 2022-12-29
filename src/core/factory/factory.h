#pragma once
#include "entt.hpp"
#include "raylib.h"

struct EntityFactory
{
    static entt::entity createBox(const Vector3& pos, const Vector3& velocity, const Vector3& halfSize, const float mass);
    static entt::entity createSphere(const Vector3& pos, const Vector3& velocity, const float radius, const float mass);
    static entt::entity createPlane(const Vector3& direction, const float offset, const float debugExtend = 90.f);
    static entt::entity createLight(Shader shader, int type, Vector3 position, Vector3 target, Color color, int casterProjType = CAMERA_ORTHOGRAPHIC, float cutoff = 0.7f, float lightRadius = 10.f, float spotSoftness = 0.65f);
};