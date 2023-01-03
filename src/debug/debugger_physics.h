#pragma once
#include "raylib.h"
#include "raymath.h"
#include "cyclone/collide_fine.h"

#include "core/system/physics/physics.h"
#include "utils/raylib_cyclone_adapter.h"

template<>
struct SystemDebugger<PhysSystem>
{
    void drawContacts() 
    {
        const auto& physSystem = PhysSystem::getSystem();

        auto* contacts = physSystem.m_contacts;
        for (int i = 0; i < physSystem.m_collisionData->contactCount; ++i) 
        {
            // Inter-body contacts are in green, floor contacts are red
            auto color = contacts[i].body[1] ? GREEN : RED;
            Vector3 pos = toRaylib(contacts[i].contactPoint);
            Vector3 normal = toRaylib(contacts[i].contactNormal);
            DrawLine3D(pos, Vector3Add(pos, normal), color);
        }
    }

    int getContactsCount()
    {
        return PhysSystem::getSystem().m_contactsCount;
    }

    int getRigidBodiesCount()
    {
        return PhysSystem::getSystem().m_rigidBodiesCount;
    }

    int getStaticBodiesCount()
    {
        return PhysSystem::getSystem().m_staticBodiesCount;
    }

    int getSleepingBodiesCount()
    {
        return PhysSystem::getSystem().m_sleepingBodiesCount;
    }

    void setUpdatePaused(bool paused)
    {
        PhysSystem::getSystem().m_isUpdatePaused = paused;
    }
};