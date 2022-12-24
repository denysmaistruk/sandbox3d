#pragma once
#include "raylib.h"
#include "raymath.h"
#include "utils/raylib_cyclone_adapter.h"
#include "core/system/physics.h"
#include "cyclone/collide_fine.h"

template<>
struct SystemDebugger<PhysSystem>
{
    void drawContacts() {
        const auto& physSystem = PhysSystem::get();

        auto* contacts = physSystem.m_contacts;
        for (int i = 0; i < physSystem.m_collisionData->contactCount; ++i) {
            // Inter-body contacts are in green, floor contacts are red
            auto color = contacts[i].body[1] ? GREEN : RED;
            Vector3 pos = toRaylib(contacts[i].contactPoint);
            Vector3 normal = toRaylib(contacts[i].contactNormal);
            DrawLine3D(pos, Vector3Add(pos, normal), color);
        }
    }
};