#pragma once
#include "entt.hpp"
#include "raylib.h"

struct EntityFactory
{
    static entt::entity createBox(const Vector3& pos, const Vector3& velocity, const Vector3& halfSize, const float mass);
    static entt::entity createSphere(const Vector3& pos, const Vector3& velocity, const float radius, const float mass);
    static entt::entity createPlane(const Vector3& direction, const float offset, const float debugExtend = 90.f);
    static entt::entity createLight(Shader shader, int type, Vector3 position, Vector3 target, Color color, int casterProjType = CAMERA_ORTHOGRAPHIC, float cutoff = 0.7f, float lightRadius = 10.f, float spotSoftness = 0.65f);
    template<typename ...Args>
    static entt::entity create(Args&& ...args);
};

template<typename ...Arg>
entt::entity EntityFactory::create(Arg&& ...arg)
{
    entt::registry& registry= EntityRegistry::getRegistry();
    entt::entity    entity  = registry.create();
    auto const __expand_seq = {
        ((void) registry.emplace<Arg>(entity, std::forward<Arg>(arg)), 0) ...
    };
    return entity;
}
