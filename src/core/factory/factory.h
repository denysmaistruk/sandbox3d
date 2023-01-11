#pragma once
#include "entt.hpp"
#include "raylib.h"

enum EState {
    eActive,
    eInactive,
};

struct EntityFactory
{
    static entt::entity createBox(const Vector3& pos, const Vector3& velocity, const Vector3& halfSize, const float mass);
    static entt::entity createSphere(const Vector3& pos, const Vector3& velocity, const float radius, const float mass);
    static entt::entity createPlane(const Vector3& direction, const float offset, const float debugExtend = 90.f);
    static entt::entity createLight(Shader shader, int type, Vector3 position, Vector3 target, Color color, int casterProjType = CAMERA_ORTHOGRAPHIC, float cutoff = 0.7f, float lightRadius = 10.f, float spotSoftness = 0.65f);
    template<typename Type, typename ...Args>
    static entt::entity createLight2(EState state, Color color, Vector3 position, Args ...a);
};

template<typename Type, typename ...Args>
entt::entity EntityFactory::createLight2(EState state, Color color, Vector3 position, Args ...a)
{
    entt::registry& registry    = EntityRegistry::getRegistry();
    entt::entity    entity      = registry.create();
    registry.emplace<Type>      (entity, a...);
    registry.emplace<Position>  (entity, position);
    registry.emplace<Color>     (entity, color);
    if (state == eInactive)
        registry.emplace<Inactive>(entity);

    return entity;
}
