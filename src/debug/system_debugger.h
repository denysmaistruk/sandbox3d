#pragma once
#include "core/system/physics.h"

class SystemDebugger
{
public:
    void drawPhysDebug_Contacts();
};

template<typename System>
class SystemDebugger2
{
public:
    void draw() const {};
};

//template<>
//class SystemDebugger2<class PhysSystem>
//{
//    void drawPhysDebug_Contacts()
//    {
//        //const auto& physSystem = PhysSystem::get();
//
//        //auto* contacts = physSystem.m_contacts;
//        //for (int i = 0; i < physSystem.m_collisionData->contactCount; ++i) {
//        //    // Inter body contacts are in green, floor contacts are red
//        //    auto color = contacts[i].body[1] ? GREEN : RED;
//        //    Vector3 pos = toRaylib(contacts[i].contactPoint);
//        //    Vector3 normal = toRaylib(contacts[i].contactNormal);
//        //    DrawLine3D(pos, Vector3Add(pos, normal), color);
//        //}
//    }
//};